/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "localcollection.h"
#include <functional>
#include <algorithm>
#include <numeric>
#include <QStandardItemModel>
#include <QtConcurrentMap>
#include <QtConcurrentRun>
#include <QTimer>
#include <QtDebug>
#include <interfaces/core/ientitymanager.h>
#include <util/sll/either.h>
#include <util/xpc/util.h>
#include <util/sll/prelude.h>
#include <util/threads/futures.h>
#include "localcollectionstorage.h"
#include "core.h"
#include "util.h"
#include "localfileresolver.h"
#include "player.h"
#include "albumartmanager.h"
#include "xmlsettingsmanager.h"
#include "localcollectionwatcher.h"
#include "localcollectionmodel.h"

namespace LC::LMP
{
	LocalCollection::LocalCollection (QObject *parent)
	: QObject (parent)
	, Storage_ (new LocalCollectionStorage (this))
	, CollectionModel_ (new LocalCollectionModel (Storage_, this))
	, FilesWatcher_ (new LocalCollectionWatcher (this))
	, AlbumArtMgr_ (new AlbumArtManager (*this, this)) // TODO it needn't be owned by the collection
	, Watcher_ (new QFutureWatcher<MediaInfo> (this))
	{
		connect (Watcher_,
				SIGNAL (finished ()),
				this,
				SLOT (handleScanFinished ()));
		connect (Watcher_,
				SIGNAL (progressValueChanged (int)),
				this,
				SIGNAL (scanProgressChanged (int)));

		Util::Sequence (this, QtConcurrent::run ([] { return LocalCollectionStorage ().Load (); })) >>
				[this] (const LocalCollectionStorage::LoadResult& result)
				{
					Storage_->Load (result);
					HandleNewArtists (result.Artists_, result.IgnoredTracks_);

					IsReady_ = true;
					emit collectionReady ();

					QTimer::singleShot (5000, this, &LocalCollection::RescanOnLoad);
				};

		auto& xsd = XmlSettingsManager::Instance ();
		QStringList oldDefault { xsd.property ("CollectionDir").toString () };
		oldDefault.removeAll ({});
		AddRootPaths (xsd.Property ("RootCollectionPaths", oldDefault).toStringList ());
		connect (this,
				SIGNAL (rootPathsChanged (QStringList)),
				this,
				SLOT (saveRootPaths ()));
	}

	bool LocalCollection::IsReady () const
	{
		return IsReady_;
	}

	AlbumArtManager* LocalCollection::GetAlbumArtManager () const
	{
		return AlbumArtMgr_;
	}

	LocalCollectionStorage* LocalCollection::GetStorage () const
	{
		return Storage_;
	}

	QAbstractItemModel* LocalCollection::GetCollectionModel () const
	{
		return CollectionModel_;
	}

	QVariant LocalCollection::GetTrackData (int trackId, LocalCollectionModel::Role role) const
	{
		return CollectionModel_->GetTrackData (trackId, role);
	}

	void LocalCollection::Clear ()
	{
		Storage_->Clear ();
		CollectionModel_->Clear ();
		Artists_.clear ();
		PresentPaths_.clear ();

		Path2Track_.clear ();
		Track2Path_.clear ();

		Track2Album_.clear ();
		AlbumID2Album_.clear ();
		AlbumID2ArtistID_.clear ();

		RemoveRootPaths (RootPaths_);
	}

	namespace
	{
		struct IterateResult
		{
			QSet<QString> UnchangedFiles_;
			QSet<QString> ChangedFiles_;
		};
	}

	void LocalCollection::Scan (const QString& path, bool root)
	{
		if (root)
			AddRootPaths ({ path });

		const bool symLinks = XmlSettingsManager::Instance ()
				.property ("FollowSymLinks").toBool ();
		auto worker = [path, symLinks]
		{
			IterateResult result;

			LocalCollectionStorage storage;

			const auto& paths = storage.GetTracksPaths ();

			const auto& allInfos = RecIterateInfo (path, symLinks);
			for (const auto& info : allInfos)
			{
				const auto& trackPath = info.absoluteFilePath ();
				const auto& mtime = info.lastModified ();
				try
				{
					const auto& storedDt = storage.GetMTime (trackPath);
					if (storedDt.isValid () &&
							std::abs (storedDt.msecsTo (mtime)) < 1500)
					{
						result.UnchangedFiles_ << trackPath;
						continue;
					}
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "error getting mtime"
							<< trackPath
							<< e.what ();
				}

				if (paths.contains (trackPath))
				{
					try
					{
						storage.SetMTime (trackPath, mtime);
					}
					catch (const std::exception& e)
					{
						qWarning () << Q_FUNC_INFO
								<< "error setting mtime"
								<< trackPath
								<< e.what ();
					}
				}
				result.ChangedFiles_ << trackPath;
			}

			return result;
		};
		Util::Sequence (this, QtConcurrent::run (worker)) >>
				[this, path] (const IterateResult& result)
				{
					CheckRemovedFiles (result.ChangedFiles_ + result.UnchangedFiles_, path);

					if (Watcher_->isRunning ())
						NewPathsQueue_ << result.ChangedFiles_;
					else
						InitiateScan (result.ChangedFiles_);
				};
	}

	void LocalCollection::Unscan (const QString& path)
	{
		if (!RootPaths_.contains (path))
			return;

		const auto& toRemove = Util::Filter (PresentPaths_,
				[&path] (const QString& subPath) { return subPath.startsWith (path); });
		PresentPaths_.subtract (toRemove);

		try
		{
			for (const auto& item : toRemove)
				RemoveTrack (item);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error unscanning"
					<< path
					<< e.what ();
			return;
		}

		RemoveRootPaths (QStringList (path));
	}

	void LocalCollection::Rescan ()
	{
		auto paths = RootPaths_;
		Clear ();

		for (const auto& path : paths)
			Scan (path, true);
	}

	LocalCollection::DirStatus LocalCollection::GetDirStatus (const QString& dir) const
	{
		if (RootPaths_.contains (dir))
			return DirStatus::RootPath;

		const auto hasSub = std::any_of (RootPaths_.begin (), RootPaths_.end (),
				[&dir] (const auto& root) { return dir.startsWith (root); });
		return hasSub ?
				DirStatus::SubPath :
				DirStatus::None;
	}

	QStringList LocalCollection::GetDirs () const
	{
		return RootPaths_;
	}

	int LocalCollection::FindArtist (const QString& artist) const
	{
		auto artistPos = std::find_if (Artists_.begin (), Artists_.end (),
				[&artist] (const auto& item) { return !QString::compare (item.Name_, artist, Qt::CaseInsensitive); });
		return artistPos == Artists_.end () ?
			-1 :
			artistPos->ID_;
	}

	int LocalCollection::FindAlbum (const QString& artist, const QString& album) const
	{
		auto artistPos = std::find_if (Artists_.begin (), Artists_.end (),
				[&artist] (const auto& item) { return !QString::compare (item.Name_, artist, Qt::CaseInsensitive); });
		if (artistPos == Artists_.end ())
		{
			qWarning () << Q_FUNC_INFO
					<< "artist not found:"
					<< artist
					<< album;
			return -1;
		}

		const auto& albums = artistPos->Albums_;
		auto albumPos = std::find_if (albums.begin (), albums.end (),
				[&album] (const auto& item) { return !QString::compare (item->Name_, album, Qt::CaseInsensitive); });
		if (albumPos == albums.end ())
		{
			qWarning () << Q_FUNC_INFO
					<< "album not found:"
					<< artist
					<< album;
			return -1;
		}

		return (*albumPos)->ID_;
	}

	void LocalCollection::SetAlbumArt (int id, const QString& path)
	{
		CollectionModel_->SetAlbumArt (id, path);

		if (AlbumID2Album_.contains (id))
			AlbumID2Album_ [id]->CoverPath_ = path;

		Storage_->SetAlbumArt (id, path);
	}

	Collection::Album_ptr LocalCollection::GetAlbum (int albumId) const
	{
		return AlbumID2Album_ [albumId];
	}

	int LocalCollection::FindTrack (const QString& path) const
	{
		return Path2Track_.value (path, -1);
	}

	int LocalCollection::GetTrackAlbumId (int trackId) const
	{
		return Track2Album_ [trackId];
	}

	Collection::Album_ptr LocalCollection::GetTrackAlbum (int trackId) const
	{
		return AlbumID2Album_ [Track2Album_ [trackId]];
	}

	QList<int> LocalCollection::GetDynamicPlaylist (DynamicPlaylist type) const
	{
		QList<int> result;
		switch (type)
		{
		case DynamicPlaylist::Random50:
		{
			const auto& keys = Track2Path_.keys ();
			if (keys.isEmpty ())
				return {};
			for (int i = 0; i < 50; ++i)
				result << keys [qrand () % keys.size ()];
			break;
		}
		case DynamicPlaylist::LovedTracks:
			result = Storage_->GetLovedTracks ();
			break;
		case DynamicPlaylist::BannedTracks:
			result = Storage_->GetBannedTracks ();
			break;
		}
		return result;
	}

	QStringList LocalCollection::TrackList2PathList (const QList<int>& tracks) const
	{
		auto result = Util::Map (tracks, [this] (int id) { return Track2Path_ [id]; });
		result.removeAll ({});
		return result;
	}

	void LocalCollection::AddTrackTo (int trackId, StaticRating rating)
	{
		switch (rating)
		{
		case StaticRating::Loved:
			Storage_->SetTrackLoved (trackId);
			break;
		case StaticRating::Banned:
			Storage_->SetTrackBanned (trackId);
			break;
		}
	}

	Collection::TrackStats LocalCollection::GetTrackStats (const QString& path) const
	{
		if (!Path2Track_.contains (path))
			return Collection::TrackStats ();

		try
		{
			return Storage_->GetTrackStats (Path2Track_ [path]);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error fetching stats for track"
					<< path
					<< Path2Track_ [path]
					<< e.what ();
			return Collection::TrackStats ();
		}
	}

	QList<int> LocalCollection::GetAlbumArtists (int albumId) const
	{
		QList<int> result;
		for (const auto& artist : Artists_)
		{
			if (std::any_of (artist.Albums_.begin (), artist.Albums_.end (),
					[albumId] (const auto& album) { return album->ID_ == albumId; }))
				result << artist.ID_;
		}
		return result;
	}

	Collection::Artist LocalCollection::GetArtist (int id) const
	{
		auto pos = std::find_if (Artists_.begin (), Artists_.end (),
				[id] (const auto& artist) { return artist.ID_ == id; });
		return pos != Artists_.end () ?
				*pos :
				Collection::Artist ();
	}

	Collection::Artists_t LocalCollection::GetAllArtists () const
	{
		return Artists_;
	}

	void LocalCollection::HandleExistingInfos (const QList<MediaInfo>& infos)
	{
		for (const auto& info : infos)
		{
			const auto& path = info.LocalPath_;
			const auto trackIdx = FindTrack (path);
			const auto trackAlbum = GetTrackAlbum (trackIdx);
			if (!trackAlbum)
			{
				qWarning () << Q_FUNC_INFO
						<< "no album for track"
						<< path;
				continue;
			}

			const auto pos = std::find_if (trackAlbum->Tracks_.begin (), trackAlbum->Tracks_.end (),
					[trackIdx] (const auto& track) { return track.ID_ == trackIdx; });
			const auto& track = pos != trackAlbum->Tracks_.end () ?
					*pos :
					Collection::Track ();
			const auto& artist = GetArtist (AlbumID2ArtistID_ [trackAlbum->ID_]);
			if (artist.Name_ == info.Artist_ &&
					trackAlbum->Name_ == info.Album_ &&
					trackAlbum->Year_ == info.Year_ &&
					track.Number_ == info.TrackNumber_ &&
					track.Name_ == info.Title_ &&
					track.Genres_ == info.Genres_)
				continue;

			auto stats = GetTrackStats (path);
			RemoveTrack (path);

			const auto& newArts = Storage_->AddToCollection ({ info });
			HandleNewArtists (newArts);

			const auto newTrackIdx = FindTrack (path);
			stats.TrackID_ = newTrackIdx;
			Storage_->SetTrackStats (stats);
		}
	}

	namespace
	{
		void DumpAlbumsSet (const QList<Collection::Album_ptr>& albumsSet, const char *context)
		{
			if (qgetenv ("LC_LMP_DEBUG_UNITE_SPLITS") != "1")
				return;

			qDebug () << context;
			qDebug () << "\t" << albumsSet [0]->Name_;
			for (const auto& album : albumsSet)
			{
				qDebug () << "\tnext album:";
				for (const auto& track : album->Tracks_)
					qDebug () << "\t\t" << track.Number_ << track.Name_ << track.Length_;
			}
		}

		uint8_t LevDist (const QString& str1, const QString& str2)
		{
			const auto len1 = str1.size ();
			const auto len2 = str2.size ();

			const auto max = std::numeric_limits<uint8_t>::max ();
			if (len1 >= max || len2 >= max)
				return max;

			std::vector<uint8_t> lastColumn (len2 + 1);

			auto prevColumn = lastColumn;
			std::iota (prevColumn.begin (), prevColumn.end (), 0);

			for (int i = 0; i < len1; ++i)
			{
				lastColumn [0] = i + 1;
				for (int j = 0; j < len2; ++j)
					lastColumn [j + 1] = std::min ({
							prevColumn [1 + j] + 1,
							lastColumn [j] + 1,
							prevColumn [j] + (str1 [i] != str2 [j])
						});

				using std::swap;
				swap (lastColumn, prevColumn);
			}

			return prevColumn [len2];
		}

		/* Note that operator== and operator< aren't fully consistent.
		 * Presumably that's OK as operator< is used to sort the list of
		 * tracks, and only then operator== is used to remove duplicates
		 * in that list. operator<'s induced equality is more strict, and
		 * if the proxied strings are compared last, chances are they are
		 * effectively equal from the user's point of view if they are next
		 * to each other in the track list, as all other fields like track
		 * number or length should match for this. In this particular case
		 * it's safe for operator== to induce a more coarse equivalence
		 * relation.
		 */
		struct LevProxy
		{
			const QString& Str_;

			bool operator== (const LevProxy& other) const
			{
				if (Str_ == other.Str_)
					return true;

				return LevDist (Str_, other.Str_) <= 3;
			}

			bool operator< (const LevProxy& other) const
			{
				return Str_ < other.Str_;
			}
		};

		bool UniteSplitTryMerge (Collection::Artists_t& artists, const QList<Collection::Album_ptr>& albumsSet)
		{
			DumpAlbumsSet (albumsSet, "initial state");

			const auto trackPred = [] (int idx)
			{
				return [idx] (const auto& album)
				{
					return std::any_of (album->Tracks_.begin (), album->Tracks_.end (),
							[idx] (const auto& track) { return track.Number_ == idx; });
				};
			};

			const auto firstTrackCounts = std::count_if (albumsSet.begin (), albumsSet.end (), trackPred (1));
			if (firstTrackCounts != 1)
			{
				qDebug () << "cannot break"
						<< firstTrackCounts
						<< "ties for albums"
						<< albumsSet [0]->Name_
						<< albumsSet [0]->Year_
						<< "\n";
				return false;
			}

			for (const auto& album : albumsSet)
			{
				const auto toTuple = [] (const auto& track)
						{ return std::make_tuple (track.Number_, track.Length_, LevProxy { track.Name_ }); };

				std::sort (album->Tracks_.begin (), album->Tracks_.end (), Util::ComparingBy (toTuple));
				const auto unique = std::unique (album->Tracks_.begin (), album->Tracks_.end (),
						Util::EqualityBy (toTuple));
				album->Tracks_.erase (unique, album->Tracks_.end ());
			}

			DumpAlbumsSet (albumsSet, "after unification");

			auto firstPos = std::find_if (albumsSet.begin (), albumsSet.end (), trackPred (1));
			while (true)
			{
				const auto curTracksCount = (*firstPos)->Tracks_.size ();
				auto nextPos = std::find_if (albumsSet.begin (), albumsSet.end (), trackPred (curTracksCount + 1));
				if (nextPos == albumsSet.end ())
					break;

				(*firstPos)->Tracks_ += (*nextPos)->Tracks_;

				for (auto& artist : artists)
				{
					const auto otherIdx = artist.Albums_.indexOf (*nextPos);
					if (otherIdx >= 0)
						artist.Albums_ [otherIdx] = *firstPos;
				}
			}

			DumpAlbumsSet (albumsSet, "after whole processing");

			return true;
		}

		void UniteSplitAlbums (Collection::Artists_t& artists)
		{
			QHash<QPair<int, QString>, QList<Collection::Album_ptr>> potentialSplitAlbums;

			for (const auto& artist : artists)
				for (const auto& album : artist.Albums_)
					potentialSplitAlbums [{ album->Year_, album->Name_ }] << album;

			for (auto it = potentialSplitAlbums.begin (); it != potentialSplitAlbums.end (); )
			{
				if (it.value ().size () == 1)
					it = potentialSplitAlbums.erase (it);
				else
					++it;
			}

			if (potentialSplitAlbums.isEmpty ())
				return;

			qDebug () << Q_FUNC_INFO
					<< "candidates:"
					<< potentialSplitAlbums.keys ();
			for (const auto& albumsSet : potentialSplitAlbums)
				UniteSplitTryMerge (artists, albumsSet);
		}
	}

	void LocalCollection::PostprocessArtistsInfos (Collection::Artists_t& artists)
	{
		UniteSplitAlbums (artists);
	}

	void LocalCollection::HandleNewArtists (Collection::Artists_t artists, const QSet<int>& ignored)
	{
		PostprocessArtistsInfos (artists);

		int albumCount = 0;
		int trackCount = 0;
		const bool shouldEmit = !Artists_.isEmpty ();

		for (const auto& artist : artists)
		{
			const auto pos = std::find_if (Artists_.begin (), Artists_.end (),
					[&artist] (const auto& present) { return present.ID_ == artist.ID_; });
			if (pos == Artists_.end ())
			{
				const auto pos = std::lower_bound (Artists_.begin (), Artists_.end (), artist,
						[] (const Collection::Artist& a1, const Collection::Artist& a2)
						{
							return QString::localeAwareCompare (a1.Name_, a2.Name_);
						});
				Artists_.insert (pos, artist);
			}
			else
				pos->Albums_ << artist.Albums_;

			for (const auto& album : artist.Albums_)
				for (const auto& track : album->Tracks_)
					PresentPaths_ << track.FilePath_;
		}

		for (const auto& artist : artists)
		{
			albumCount += artist.Albums_.size ();
			for (const auto& album : artist.Albums_)
			{
				trackCount += album->Tracks_.size ();

				auto& presentAlbum = AlbumID2Album_ [album->ID_];
				if (!presentAlbum)
				{
					presentAlbum = album;
					AlbumID2ArtistID_ [album->ID_] = artist.ID_;
				}
				else if (presentAlbum != album)
					presentAlbum->Tracks_ << album->Tracks_;

				for (const auto& track : album->Tracks_)
				{
					Path2Track_ [track.FilePath_] = track.ID_;
					Track2Path_ [track.ID_] = track.FilePath_;

					Track2Album_ [track.ID_] = album->ID_;
				}
			}
		}

		CollectionModel_->AddArtists (artists);

		for (const auto item : ignored)
			CollectionModel_->IgnoreTrack (item);

		if (shouldEmit &&
				trackCount)
		{
			UpdateNewArtists_ += artists.size ();
			UpdateNewAlbums_ += albumCount;
			UpdateNewTracks_ += trackCount;

			emit gotNewArtists (artists);
		}
	}

	void LocalCollection::IgnoreTrack (const QString& path)
	{
		const int id = FindTrack (path);
		if (id == -1)
			return;

		try
		{
			Storage_->IgnoreTrack (id);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error removing track:"
					<< e.what ();
			throw;
		}

		CollectionModel_->IgnoreTrack (id);
	}

	void LocalCollection::RemoveTrack (const QString& path)
	{
		const int id = FindTrack (path);
		if (id == -1)
			return;

		auto album = GetTrackAlbum (id);
		try
		{
			Storage_->RemoveTrack (id);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error removing track:"
					<< e.what ();
			throw;
		}

		CollectionModel_->RemoveTrack (id);

		Path2Track_.remove (path);
		Track2Path_.remove (id);
		Track2Album_.remove (id);
		PresentPaths_.remove (path);

		if (!album)
			return;

		auto pos = std::remove_if (album->Tracks_.begin (), album->Tracks_.end (),
				[id] (const auto& item) { return item.ID_ == id; });
		album->Tracks_.erase (pos, album->Tracks_.end ());

		if (album->Tracks_.isEmpty ())
			RemoveAlbum (album->ID_);
	}

	void LocalCollection::RemoveAlbum (int id)
	{
		try
		{
			Storage_->RemoveAlbum (id);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error removing album:"
					<< e.what ();
			throw;
		}

		AlbumID2Album_.remove (id);
		AlbumID2ArtistID_.remove (id);

		CollectionModel_->RemoveAlbum (id);

		for (auto i = Artists_.begin (); i != Artists_.end (); )
		{
			auto& artist = *i;

			auto pos = std::find_if (artist.Albums_.begin (), artist.Albums_.end (),
					[id] (const auto& album) { return album->ID_ == id; });
			if (pos == artist.Albums_.end ())
			{
				++i;
				continue;
			}

			artist.Albums_.erase (pos);
			if (artist.Albums_.isEmpty ())
				i = RemoveArtist (i);
			else
				++i;
		}
	}

	Collection::Artists_t::iterator LocalCollection::RemoveArtist (Collection::Artists_t::iterator pos)
	{
		const int id = pos->ID_;
		try
		{
			Storage_->RemoveArtist (id);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error removing artist:"
					<< e.what ();
			throw;
		}

		CollectionModel_->RemoveArtist (id);
		return Artists_.erase (pos);
	}

	void LocalCollection::AddRootPaths (QStringList paths)
	{
		for (const auto& path : RootPaths_)
			paths.removeAll (path);
		if (paths.isEmpty ())
			return;

		RootPaths_ << paths;
		emit rootPathsChanged (RootPaths_);

		for (const auto& path : paths)
			FilesWatcher_->AddPath (path);
	}

	void LocalCollection::RemoveRootPaths (const QStringList& paths)
	{
		int removed = 0;
		for (const auto& str : paths)
		{
			removed += RootPaths_.removeAll (str);
			FilesWatcher_->RemovePath (str);
		}

		if (removed)
			emit rootPathsChanged (RootPaths_);
	}

	void LocalCollection::CheckRemovedFiles (const QSet<QString>& scanned, const QString& rootPath)
	{
		auto toRemove = PresentPaths_;
		toRemove.subtract (scanned);

		for (auto pos = toRemove.begin (); pos != toRemove.end (); )
		{
			if (pos->startsWith (rootPath))
				++pos;
			else
				pos = toRemove.erase (pos);
		}

		for (const auto& path : toRemove)
			RemoveTrack (path);
	}

	void LocalCollection::InitiateScan (const QSet<QString>& newPaths)
	{
		auto resolver = Core::Instance ().GetLocalFileResolver ();

		emit scanStarted (newPaths.size ());
		auto worker = [resolver] (const QString& path)
		{
			return resolver->ResolveInfo (path).ToRight ([] (const ResolveError& error)
					{
						qWarning () << Q_FUNC_INFO
								<< "error resolving media info for"
								<< error.FilePath_
								<< error.ReasonString_;
						return MediaInfo {};
					});
		};
		const auto& future = QtConcurrent::mapped (newPaths,
				std::function<MediaInfo (QString)> (worker));
		Watcher_->setFuture (future);
	}

	void LocalCollection::RecordPlayedTrack (const QString& path)
	{
		if (Path2Track_.contains (path))
			RecordPlayedTrack (Path2Track_ [path], QDateTime::currentDateTime ());
	}

	void LocalCollection::RecordPlayedTrack (int trackId, const QDateTime& date)
	{
		try
		{
			CollectionModel_->UpdatePlayStats (trackId);
			Storage_->RecordTrackPlayed (trackId, date);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error recording played info for track"
					<< e.what ();
		}
	}

	void LocalCollection::RescanOnLoad ()
	{
		for (const auto& rootPath : RootPaths_)
			Scan (rootPath, true);
	}

	void LocalCollection::handleScanFinished ()
	{
		auto future = Watcher_->future ();
		QList<MediaInfo> newInfos, existingInfos;
		for (const auto& info : future)
		{
			const auto& path = info.LocalPath_;
			if (path.isEmpty ())
				continue;

			if (PresentPaths_.contains (path))
				existingInfos << info;
			else
			{
				newInfos << info;
				PresentPaths_ += path;
			}
		}

		emit scanFinished ();

		auto newArts = Storage_->AddToCollection (newInfos);
		HandleNewArtists (newArts);

		if (!NewPathsQueue_.isEmpty ())
			InitiateScan (NewPathsQueue_.takeFirst ());
		else if (UpdateNewTracks_)
		{
			const auto& artistsMsg = tr ("%n new artist(s)", 0, UpdateNewArtists_);
			const auto& albumsMsg = tr ("%n new album(s)", 0, UpdateNewAlbums_);
			const auto& tracksMsg = tr ("%n new track(s)", 0, UpdateNewTracks_);
			const auto& msg = tr ("Local collection updated: %1, %2, %3.")
					.arg (artistsMsg)
					.arg (albumsMsg)
					.arg (tracksMsg);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (Util::MakeNotification ("LMP", msg, Priority::Info));

			UpdateNewArtists_ = UpdateNewAlbums_ = UpdateNewTracks_ = 0;
		}

		HandleExistingInfos (existingInfos);
	}

	void LocalCollection::saveRootPaths ()
	{
		XmlSettingsManager::Instance ().setProperty ("RootCollectionPaths", RootPaths_);
	}
}
