/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "localcollection.h"
#include <functional>
#include <algorithm>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMimeData>
#include <QtConcurrentMap>
#include <QtConcurrentRun>
#include <QTimer>
#include <QtDebug>
#include <util/util.h>
#include "localcollectionstorage.h"
#include "core.h"
#include "util.h"
#include "localfileresolver.h"
#include "player.h"
#include "albumartmanager.h"
#include "xmlsettingsmanager.h"
#include "localcollectionwatcher.h"
#include "collectionsortermodel.h"

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		QStringList CollectPaths (const QModelIndex& index, const QAbstractItemModel *model)
		{
			const auto type = index.data (LocalCollection::Role::Node).toInt ();
			if (type == LocalCollection::NodeType::Track)
				return QStringList (index.data (LocalCollection::Role::TrackPath).toString ());

			QStringList paths;
			for (int i = 0; i < model->rowCount (index); ++i)
				paths += CollectPaths (model->index (i, 0, index), model);
			return paths;
		}

		class CollectionDraggableModel : public CollectionSorterModel
		{
		public:
			CollectionDraggableModel (LocalCollection *parent)
			: CollectionSorterModel (parent)
			{
				setSupportedDragActions (Qt::CopyAction);
			}

			QStringList mimeTypes () const
			{
				return QStringList ("text/uri-list");
			}

			QMimeData* mimeData (const QModelIndexList& indexes) const
			{
				QMimeData *result = new QMimeData;

				QList<QUrl> urls;
				Q_FOREACH (const auto& index, indexes)
				{
					const auto& paths = CollectPaths (index, this);
					std::transform (paths.begin (), paths.end (), std::back_inserter (urls),
							[] (const QString& path) { return QUrl::fromLocalFile (path); });
				}

				result->setUrls (urls);

				return result;
			}
		};
	}

	LocalCollection::LocalCollection (QObject *parent)
	: QObject (parent)
	, IsReady_ (false)
	, Storage_ (new LocalCollectionStorage (this))
	, CollectionModel_ (new QStandardItemModel (this))
	, Sorter_ (new CollectionDraggableModel (this))
	, FilesWatcher_ (new LocalCollectionWatcher (this))
	, AlbumArtMgr_ (new AlbumArtManager (this))
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

		auto loadWatcher = new QFutureWatcher<LocalCollectionStorage::LoadResult> ();
		connect (loadWatcher,
				SIGNAL (finished ()),
				this,
				SLOT (handleLoadFinished ()));
		auto worker = [] () { return LocalCollectionStorage ().Load (); };
		auto future = QtConcurrent::run (std::function<LocalCollectionStorage::LoadResult ()> (worker));
		loadWatcher->setFuture (future);

		auto& xsd = XmlSettingsManager::Instance ();
		QStringList oldDefault (xsd.property ("CollectionDir").toString ());
		oldDefault.removeAll (QString ());
		AddRootPaths (xsd.Property ("RootCollectionPaths", oldDefault).toStringList ());
		connect (this,
				SIGNAL (rootPathsChanged (QStringList)),
				this,
				SLOT (saveRootPaths ()));

		Sorter_->setSourceModel (CollectionModel_);
		Sorter_->setDynamicSortFilter (true);
		Sorter_->sort (0);
	}

	void LocalCollection::FinalizeInit ()
	{
		ArtistIcon_ = Core::Instance ().GetProxy ()->GetIcon ("view-media-artist");
	}

	bool LocalCollection::IsReady () const
	{
		return IsReady_;
	}

	QAbstractItemModel* LocalCollection::GetCollectionModel () const
	{
		return Sorter_;
	}

	void LocalCollection::Enqueue (const QModelIndex& index, Player *player)
	{
		player->Enqueue (CollectPaths (index, Sorter_));
	}

	void LocalCollection::Enqueue (const QList<QModelIndex>& indexes, Player *player)
	{
		const auto& paths = std::accumulate (indexes.begin (), indexes.end (), QStringList (),
				[this] (const QStringList& paths, decltype (indexes.front ()) item)
					{ return paths + CollectPaths (item, Sorter_); });
		qDebug () << Q_FUNC_INFO << indexes << paths;
		player->Enqueue (paths);
	}

	void LocalCollection::Clear ()
	{
		Storage_->Clear ();
		CollectionModel_->clear ();
		PresentPaths_.clear ();
		Artist2Item_.clear ();
		Album2Item_.clear ();

		RemoveRootPaths (RootPaths_);
	}

	void LocalCollection::Scan (const QString& path, bool root)
	{
		auto watcher = new QFutureWatcher<QStringList> ();
		connect (watcher,
				SIGNAL (finished ()),
				this,
				SLOT (handleIterateFinished ()));
		watcher->setProperty ("Path", path);
		watcher->setProperty ("IsRoot", root);

		if (root)
			AddRootPaths (QStringList (path));

		const bool symLinks = XmlSettingsManager::Instance ()
				.property ("FollowSymLinks").toBool ();
		watcher->setFuture (QtConcurrent::run (RecIterate, path, symLinks));
	}

	void LocalCollection::Unscan (const QString& path)
	{
		if (!RootPaths_.contains (path))
			return;

		QStringList toRemove;
		auto pred = [&path] (const QString& subPath) { return subPath.startsWith (path); };
		std::copy_if (PresentPaths_.begin (), PresentPaths_.end (),
				std::back_inserter (toRemove), pred);
		PresentPaths_.subtract (QSet<QString>::fromList (toRemove));

		try
		{
			std::for_each (toRemove.begin (), toRemove.end (),
					[this] (const QString& path) { RemoveTrack (path); });
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

		Q_FOREACH (const auto& path, paths)
			Scan (path, true);
	}

	LocalCollection::DirStatus LocalCollection::GetDirStatus (const QString& dir) const
	{
		if (RootPaths_.contains (dir))
			return DirStatus::RootPath;

		auto pos = std::find_if (RootPaths_.begin (), RootPaths_.end (),
				[&dir] (decltype (RootPaths_.front ()) root) { return dir.startsWith (root); });
		return pos == RootPaths_.end () ?
				DirStatus::None :
				DirStatus::SubPath;
	}

	QStringList LocalCollection::GetDirs () const
	{
		return RootPaths_;
	}

	int LocalCollection::FindArtist (const QString& artist) const
	{
		auto artistPos = std::find_if (Artists_.begin (), Artists_.end (),
				[&artist] (decltype (Artists_.front ()) item) { return item.Name_ == artist; });
		return artistPos == Artists_.end () ?
			-1 :
			artistPos->ID_;
	}

	int LocalCollection::FindAlbum (const QString& artist, const QString& album) const
	{
		auto artistPos = std::find_if (Artists_.begin (), Artists_.end (),
				[&artist] (decltype (Artists_.front ()) item) { return item.Name_ == artist; });
		if (artistPos == Artists_.end ())
			return -1;

		auto albumPos = std::find_if (artistPos->Albums_.begin (), artistPos->Albums_.end (),
				[&album] (decltype (artistPos->Albums_.front ()) item) { return item->Name_ == album; });
		if (albumPos == artistPos->Albums_.end ())
			return -1;

		return (*albumPos)->ID_;
	}

	void LocalCollection::SetAlbumArt (int id, const QString& path)
	{
		if (Album2Item_.contains (id))
			Album2Item_ [id]->setData (path, Role::AlbumArt);

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

	Collection::Album_ptr LocalCollection::GetTrackAlbum (int trackId) const
	{
		return AlbumID2Album_ [Track2Album_ [trackId]];
	}

	QList<int> LocalCollection::GetDynamicPlaylist (DynamicPlaylist type) const
	{
		QList<int> result;
		const auto& keys = Track2Path_.keys ();
		switch (type)
		{
		case DynamicPlaylist::Random50:
			for (int i = 0; i < 50; ++i)
				result << keys [qrand () % keys.size ()];
			break;
		}
		return result;
	}

	QStringList LocalCollection::TrackList2PathList (const QList<int>& tracks) const
	{
		QStringList result;
		std::transform (tracks.begin (), tracks.end (), std::back_inserter (result),
				[this] (int id) { return Track2Path_ [id]; });
		result.removeAll (QString ());
		return result;
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

	Collection::Artists_t LocalCollection::GetAllArtists () const
	{
		return Artists_;
	}

	namespace
	{
		template<typename T, typename U, typename Init, typename Parent>
		QStandardItem* GetItem (T& c, U idx, Init f, Parent parent)
		{
			auto item = c [idx];
			if (item)
				return item;

			item = new QStandardItem ();
			item->setEditable (false);
			f (item);
			parent->appendRow (item);
			c [idx] = item;
			return item;
		}
	}

	void LocalCollection::HandleNewArtists (const Collection::Artists_t& artists)
	{
		int albumCount = 0;
		int trackCount = 0;
		const bool shouldEmit = !Artists_.isEmpty ();

		Artists_ += artists;
		Q_FOREACH (const auto& artist, artists)
			Q_FOREACH (auto album, artist.Albums_)
				Q_FOREACH (const auto& track, album->Tracks_)
					PresentPaths_ << track.FilePath_;

		Q_FOREACH (const auto& artist, artists)
		{
			albumCount += artist.Albums_.size ();

			auto artistItem = GetItem (Artist2Item_,
					artist.ID_,
					[this, &artist] (QStandardItem *item)
					{
						item->setIcon (ArtistIcon_);
						item->setText (artist.Name_);
						item->setData (artist.Name_, Role::ArtistName);
						item->setData (NodeType::Artist, Role::Node);
					},
					CollectionModel_);
			Q_FOREACH (auto album, artist.Albums_)
			{
				trackCount += album->Tracks_.size ();

				AlbumArtMgr_->CheckAlbumArt (artist, album);

				auto albumItem = GetItem (Album2Item_,
						album->ID_,
						[album] (QStandardItem *item)
						{
							item->setText (QString::fromUtf8 ("%1 — %2")
									.arg (album->Year_)
									.arg (album->Name_));
							item->setData (album->Year_, Role::AlbumYear);
							item->setData (album->Name_, Role::AlbumName);
							item->setData (NodeType::Album, Role::Node);
							if (!album->CoverPath_.isEmpty ())
								item->setData (album->CoverPath_, Role::AlbumArt);
						},
						artistItem);

				AlbumID2Album_ [album->ID_] = album;

				Q_FOREACH (const auto& track, album->Tracks_)
				{
					const QString& name = QString::fromUtf8 ("%1 — %2")
							.arg (track.Number_)
							.arg (track.Name_);
					auto item = new QStandardItem (name);
					item->setEditable (false);
					item->setData (track.Number_, Role::TrackNumber);
					item->setData (track.Name_, Role::TrackTitle);
					item->setData (track.FilePath_, Role::TrackPath);
					item->setData (NodeType::Track, Role::Node);
					albumItem->appendRow (item);

					Path2Track_ [track.FilePath_] = track.ID_;
					Track2Path_ [track.ID_] = track.FilePath_;

					Track2Album_ [track.ID_] = album->ID_;

					Track2Item_ [track.ID_] = item;
				}
			}
		}

		if (shouldEmit &&
				artists.size () &&
				albumCount &&
				trackCount)
		{
			const auto& artistsMsg = tr ("%n new artist(s)", 0, artists.size ());
			const auto& albumsMsg = tr ("%n new album(s)", 0, albumCount);
			const auto& tracksMsg = tr ("%n new track(s)", 0, trackCount);
			const auto& msg = tr ("Local collection updated: %1, %2, %3.")
					.arg (artistsMsg)
					.arg (albumsMsg)
					.arg (tracksMsg);
			Core::Instance ().SendEntity (Util::MakeNotification ("LMP", msg, PInfo_));
		}
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

		auto item = Track2Item_.take (id);
		item->parent ()->removeRow (item->row ());

		Path2Track_.remove (path);
		Track2Path_.remove (id);
		Track2Album_.remove (id);

		if (!album)
			return;

		auto pos = std::remove_if (album->Tracks_.begin (), album->Tracks_.end (),
				[id] (decltype (album->Tracks_.front ()) item) { return item.ID_ == id; });
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

		auto item = Album2Item_.take (id);
		item->parent ()->removeRow (item->row ());

		for (auto i = Artists_.begin (); i != Artists_.end (); )
		{
			auto& artist = *i;

			auto pos = std::find_if (artist.Albums_.begin (), artist.Albums_.end (),
					[id] (decltype (artist.Albums_.front ()) album) { return album->ID_ == id; });
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

		CollectionModel_->removeRow (Artist2Item_.take (id)->row ());
		return Artists_.erase (pos);
	}

	void LocalCollection::AddRootPaths (QStringList paths)
	{
		Q_FOREACH (const auto& path, RootPaths_)
			paths.removeAll (path);
		if (paths.isEmpty ())
			return;

		RootPaths_ << paths;
		emit rootPathsChanged (RootPaths_);

		std::for_each (paths.begin (), paths.end (),
				[this] (decltype (paths.front ()) item) { FilesWatcher_->AddPath (item); });
	}

	void LocalCollection::RemoveRootPaths (const QStringList& paths)
	{
		int removed = 0;
		Q_FOREACH (const auto& str, paths)
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
			if (pos->startsWith (rootPath) && !scanned.contains (*pos))
				++pos;
			else
				pos = toRemove.erase (pos);
		}

		Q_FOREACH (const auto& path, toRemove)
			RemoveTrack (path);
	}

	void LocalCollection::InitiateScan (const QSet<QString>& newPaths)
	{
		auto resolver = Core::Instance ().GetLocalFileResolver ();

		emit scanStarted (newPaths.size ());
		auto worker = [resolver] (const QString& path)
		{
			try
			{
				return resolver->ResolveInfo (path);
			}
			catch (const ResolveError& error)
			{
				qWarning () << Q_FUNC_INFO
						<< "error resolving media info for"
						<< error.GetPath ()
						<< error.what ();
				return MediaInfo ();
			}
		};
		QFuture<MediaInfo> future = QtConcurrent::mapped (newPaths,
				std::function<MediaInfo (const QString&)> (worker));
		Watcher_->setFuture (future);
	}

	void LocalCollection::recordPlayedTrack (const QString& path)
	{
		if (!Path2Track_.contains (path))
			return;

		try
		{
			Storage_->RecordTrackPlayed (Path2Track_ [path]);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error recording played info for track"
					<< e.what ();
		}
	}

	void LocalCollection::rescanOnLoad ()
	{
		Q_FOREACH (const auto& rootPath, RootPaths_)
			Scan (rootPath, true);
	}

	void LocalCollection::handleLoadFinished ()
	{
		auto watcher = dynamic_cast<QFutureWatcher<LocalCollectionStorage::LoadResult>*> (sender ());
		watcher->deleteLater ();
		const auto& result = watcher->result ();
		Storage_->Load (result);

		HandleNewArtists (result.Artists_);

		IsReady_ = true;

		emit collectionReady ();

		QTimer::singleShot (5000,
				this,
				SLOT (rescanOnLoad ()));
	}

	void LocalCollection::handleIterateFinished ()
	{
		sender ()->deleteLater ();

		const auto& path = sender ()->property ("Path").toString ();

		auto watcher = dynamic_cast<QFutureWatcher<QStringList>*> (sender ());

		const auto& origPaths = QSet<QString>::fromList (watcher->result ());
		CheckRemovedFiles (origPaths, path);

		auto newPaths = origPaths;
		newPaths.subtract (PresentPaths_);
		if (newPaths.isEmpty ())
			return;

		if (Watcher_->isRunning ())
			NewPathsQueue_ << newPaths;
		else
			InitiateScan (newPaths);
	}

	void LocalCollection::handleScanFinished ()
	{
		auto future = Watcher_->future ();
		QList<MediaInfo> infos;
		std::copy_if (future.begin (), future.end (), std::back_inserter (infos),
				[] (const MediaInfo& info) { return !info.LocalPath_.isEmpty (); });
		Q_FOREACH (const auto& info, infos)
			PresentPaths_ += info.LocalPath_;

		emit scanFinished ();

		auto newArts = Storage_->AddToCollection (infos);
		HandleNewArtists (newArts);

		if (!NewPathsQueue_.isEmpty ())
			InitiateScan (NewPathsQueue_.takeFirst ());
	}

	void LocalCollection::saveRootPaths ()
	{
		XmlSettingsManager::Instance ().setProperty ("RootCollectionPaths", RootPaths_);
	}
}
}
