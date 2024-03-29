/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "localcollectionmodel.h"
#include <numeric>
#include <QUrl>
#include <QMimeData>
#include <QtDebug>
#include <interfaces/core/iiconthememanager.h>
#include <util/gui/util.h>
#include <util/sll/prelude.h>
#include <util/sll/unreachable.h>
#include "localcollectionstorage.h"
#include "localcollection.h"
#include "util.h"

namespace LC::LMP
{
	LocalCollectionModel::LocalCollectionModel (const Collection::Artists_t& artists,
			LocalCollectionStorage& storage, LocalCollection& collection)
	: DndActionsMixin<QAbstractItemModel> { &collection }
	, Artists_ { artists }
	, Storage_ { storage }
	, Collection_ { collection }
	{
		setSupportedDragActions (Qt::CopyAction);
	}

	QStringList LocalCollectionModel::mimeTypes () const
	{
		return { "text/uri-list" };
	}

	namespace
	{
		template<typename T, LocalCollectionModel::Role Role>
		QList<T> CollectTrackValues (const QModelIndex& index, const QAbstractItemModel *model)
		{
			const auto type = index.data (LocalCollectionModel::Role::Node).toInt ();
			if (type == LocalCollectionModel::NodeType::Track)
				return { index.data (Role).value<T> () };

			QList<T> result;
			for (int i = 0; i < model->rowCount (index); ++i)
				result += CollectTrackValues<T, Role> (model->index (i, 0, index), model);
			return result;
		}

		QStringList CollectPaths (const QModelIndex& index, const QAbstractItemModel *model)
		{
			return CollectTrackValues<QString, LocalCollectionModel::Role::TrackPath> (index, model);
		}
	}

	QMimeData* LocalCollectionModel::mimeData (const QModelIndexList& indexes) const
	{
		QList<QUrl> urls;
		for (const auto& index : indexes)
			urls += Util::Map (CollectPaths (index, this), &QUrl::fromLocalFile);

		if (urls.isEmpty ())
			return nullptr;

		auto result = new QMimeData;
		result->setUrls (urls);
		return result;
	}

	int LocalCollectionModel::columnCount (const QModelIndex&) const
	{
		return 1;
	}

	namespace
	{
		/* Upper half of the index's internal pointer contains the album index,
		 * and the lower half contains the artist index.
		 */
		constexpr auto AlbumShift = (sizeof (quintptr) / 2) * 8;
		constexpr auto ArtistMask = (quintptr { 1 } << AlbumShift) - 1;
		constexpr auto InvalidIdx = static_cast<size_t> (-1);

		bool IsArtist (quintptr id)
		{
			return !id;
		}

		bool IsAlbum (quintptr id)
		{
			return !IsArtist (id) && !(id >> AlbumShift);
		}

		bool IsTrack (quintptr id)
		{
			return id >= (quintptr { 1 } << AlbumShift);
		}

		size_t ArtistIdx (const QModelIndex& index)
		{
			if (IsArtist (index.internalId ()))
				return index.row ();

			return (index.internalId () & ArtistMask) - 1U;
		}

		size_t AlbumIdx (const QModelIndex& index)
		{
			const auto id = index.internalId ();
			if (IsAlbum (id))
				return index.row ();

			return (index.internalId () >> AlbumShift) - 1U;
		}

		size_t TrackIdx (const QModelIndex& index)
		{
			return IsTrack (index.internalId ()) ? index.row () : InvalidIdx;
		}
	}

	QModelIndex LocalCollectionModel::MakeArtistIndex (quintptr artistIdx) const
	{
		return createIndex (artistIdx, 0, quintptr { 0 });
	}

	QModelIndex LocalCollectionModel::MakeAlbumIndex (quintptr artistIdx, quintptr albumIdx) const
	{
		return createIndex (albumIdx, 0, artistIdx + 1);
	}

	QModelIndex LocalCollectionModel::MakeTrackIndex (quintptr artistIdx, quintptr albumIdx, quintptr trackIdx) const
	{
		return createIndex (trackIdx, 0, (artistIdx + 1) | (albumIdx + 1) << AlbumShift);
	}

	QVariant LocalCollectionModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return {};

		const auto artistIdx = ArtistIdx (index);
		const auto albumIdx = AlbumIdx (index);
		const auto trackIdx = TrackIdx (index);

		const auto& artist = Artists_.value (artistIdx);
		const auto& album = artist.Albums_.value (albumIdx);
		const auto track = trackIdx != InvalidIdx ?
				album->Tracks_.value (trackIdx) :
				Collection::Track {};

		const auto nodeType = [&]
		{
			if (trackIdx != InvalidIdx)
				return NodeType::Track;
			if (albumIdx != InvalidIdx)
				return NodeType::Album;
			return NodeType::Artist;
		} ();

		switch (role)
		{
		case Qt::DisplayRole:
			switch (nodeType)
			{
			case NodeType::Artist:
				return artist.Name_;
			case NodeType::Album:
				return QStringLiteral ("%1 — %2")
						.arg (album->Year_)
						.arg (album->Name_);
			case NodeType::Track:
				return QStringLiteral ("%1 — %2")
						.arg (track.Number_)
						.arg (track.Name_);
			}
			Util::Unreachable ();

		case Qt::DecorationRole:
			return nodeType == NodeType::Artist ?
					ArtistIcon_ :
					QVariant {};
		case Qt::ToolTipRole:
			switch (nodeType)
			{
			case NodeType::Artist:
				return GetArtistTooltip (artist.ID_);
			case NodeType::Album:
				return GetAlbumTooltip (album->ID_);
			case NodeType::Track:
				return GetTrackTooltip (track.ID_);
			}
			Util::Unreachable ();

		case Role::Node:
			return nodeType;

		case Role::ArtistName:
			return artist.Name_;

		case Role::AlbumID:
			return album ? album->ID_ : QVariant {};
		case Role::AlbumYear:
			return album ? album->Year_ : QVariant {};
		case Role::AlbumName:
			return album ? album->Name_ : QVariant {};
		case Role::AlbumArt:
			return album ? album->CoverPath_ : QVariant {};

		case Role::TrackID:
			return track.ID_;
		case Role::TrackNumber:
			return track.Number_;
		case Role::TrackTitle:
			return track.Name_;
		case Role::TrackPath:
			return track.FilePath_;
		case Role::TrackGenres:
			return track.Genres_;
		case Role::TrackLength:
			return track.Length_;

		case Role::IsIgnored:
			switch (nodeType)
			{
			case NodeType::Artist:
				return IgnoredArtists_.contains (artist.ID_);
			case NodeType::Album:
				return IgnoredAlbums_.contains (album->ID_);
			case NodeType::Track:
				return IgnoredTracks_.contains (track.ID_);
			}

			Util::Unreachable ();
		}

		return {};
	}

	Qt::ItemFlags LocalCollectionModel::flags (const QModelIndex& idx) const
	{
		auto flags = Qt::ItemIsSelectable
				| Qt::ItemIsEnabled
				| Qt::ItemIsDragEnabled;
		if (IsTrack (idx.internalId ()))
			flags |= Qt::ItemNeverHasChildren;
		return flags;
	}

	QModelIndex LocalCollectionModel::index (int row, int, const QModelIndex& parent) const
	{
		const auto artistIdx = ArtistIdx (parent);
		const auto albumIdx = AlbumIdx (parent);

		if (albumIdx != InvalidIdx)
			return MakeTrackIndex (artistIdx, albumIdx, row);
		if (artistIdx != InvalidIdx)
			return MakeAlbumIndex (artistIdx, row);

		return MakeArtistIndex (row);
	}

	QModelIndex LocalCollectionModel::parent (const QModelIndex& index) const
	{
		const auto artistIdx = ArtistIdx (index);
		const auto albumIdx = AlbumIdx (index);
		const auto trackIdx = TrackIdx (index);

		if (trackIdx != InvalidIdx)
			return MakeAlbumIndex (artistIdx, albumIdx);
		if (albumIdx != InvalidIdx)
			return MakeArtistIndex (artistIdx);

		return {};
	}

	int LocalCollectionModel::rowCount (const QModelIndex& index) const
	{
		const auto artistIdx = ArtistIdx (index);
		const auto albumIdx = AlbumIdx (index);
		const auto trackIdx = TrackIdx (index);

		if (trackIdx != InvalidIdx)
			return 0;
		if (albumIdx != InvalidIdx)
			return Artists_.value (artistIdx).Albums_.value (albumIdx)->Tracks_.size ();
		if (artistIdx != InvalidIdx)
			return Artists_.value (artistIdx).Albums_.size ();
		return Artists_.size ();
	}

	QList<QUrl> LocalCollectionModel::ToSourceUrls (const QList<QModelIndex>& indexes) const
	{
		const auto& paths = std::accumulate (indexes.begin (), indexes.end (), QStringList {},
				[this] (const QStringList& paths, const QModelIndex& item)
					{ return paths + CollectPaths (item, this); });

		QList<QUrl> result;
		result.reserve (paths.size ());
		for (const auto& path : paths)
			result << QUrl::fromLocalFile (path);
		return result;
	}

	Util::DefaultScopeGuard LocalCollectionModel::ResetArtists ()
	{
		beginResetModel ();

		IgnoredArtists_.clear ();
		IgnoredAlbums_.clear ();
		IgnoredTracks_.clear ();

		ArtistTooltips_.clear ();
		AlbumTooltips_.clear ();
		TrackTooltips_.clear ();

		return Util::MakeScopeGuard ([this] { endResetModel (); });
	}

	Util::DefaultScopeGuard LocalCollectionModel::InsertArtist (int idx)
	{
		beginInsertRows ({}, idx, idx);
		return EndInsertRowsGuard ();
	}

	Util::DefaultScopeGuard LocalCollectionModel::AppendAlbums (int artistIdx, int newAlbumsCount)
	{
		const auto curAlbumsCount = Artists_ [artistIdx].Albums_.size ();
		beginInsertRows (MakeArtistIndex (artistIdx), curAlbumsCount, curAlbumsCount + newAlbumsCount - 1);
		return EndInsertRowsGuard ();
	}

	namespace
	{
		int FindIdx (const Collection::Artists_t& artists, int artistId)
		{
			const auto artistPos = std::find_if (artists.begin (), artists.end (),
					[&] (const auto& artist) { return artist.ID_ == artistId; });
			if (artistPos == artists.end ())
			{
				qWarning () << "unable to find artist"
						<< artistId;
				return -1;
			}

			return artistPos - artists.begin ();
		}

		std::tuple<int, int> FindIdx (const Collection::Artists_t& artists, int artistId, int albumId)
		{
			const auto artistIdx = FindIdx (artists, artistId);
			if (artistIdx == -1)
				return { -1, -1 };

			const auto& albums = artists.at (artistIdx).Albums_;
			const auto albumPos = std::find_if (albums.begin (), albums.end (),
					[&] (const auto& album) { return album->ID_ == albumId; });
			if (albumPos == albums.end ())
			{
				qWarning () << "unable to find album"
						<< albumId
						<< "in artist"
						<< artistId;
				return { -1, -1 };
			}

			return { artistIdx, albumPos - albums.begin () };
		}

		std::tuple<int, int, int> FindIdx (const Collection::Artists_t& artists, int artistId, int albumId, int trackId)
		{
			const auto [artistIdx, albumIdx] = FindIdx (artists, artistId, albumId);
			if (artistIdx == -1 || albumIdx == -1)
				return { -1, -1, -1 };

			const auto& tracks = artists [artistIdx].Albums_ [albumIdx]->Tracks_;
			const auto trackPos = std::find_if (tracks.begin (), tracks.end (),
					[&] (const auto& track) { return track.ID_ == trackId; });
			if (trackPos == tracks.end ())
			{
				qWarning () << "unable to find track"
						<< trackId
						<< "in"
						<< albumId
						<< artistId;
				return { -1, -1, -1 };
			}

			return { artistIdx, albumIdx, trackPos - tracks.begin () };
		}
	}

	void LocalCollectionModel::IgnoreTracks (const QSet<int>& tracksIds, bool emitting)
	{
		IgnoredTracks_.unite (tracksIds);

		QSet<QPair<int, int>> albums2check;
		QSet<int> artists2check;
		for (const auto trackId : tracksIds)
			if (const auto info = Collection_.GetTrackInfo (trackId))
			{
				const auto artistId = info->Artist_.ID_;
				const auto [artistIdx, albumIdx, trackIdx] = FindIdx (Artists_, artistId, info->Album_->ID_, trackId);

				if (emitting)
				{
					const auto modelIdx = MakeTrackIndex (artistIdx, albumIdx, trackIdx);
					emit dataChanged (modelIdx, modelIdx);
				}

				albums2check.insert ({ artistIdx, albumIdx });
				artists2check.insert (artistIdx);
			}

		for (const auto [artistIdx, albumIdx] : albums2check)
		{
			const auto& album = Artists_ [artistIdx].Albums_ [albumIdx];
			if (std::all_of (album->Tracks_.begin (), album->Tracks_.end (),
					[this] (const Collection::Track& track) { return IgnoredTracks_.contains (track.ID_); }))
			{
				IgnoredAlbums_ << album->ID_;

				if (emitting)
				{
					const auto modelIdx = MakeAlbumIndex (artistIdx, albumIdx);
					emit dataChanged (modelIdx, modelIdx);
				}
			}
		}

		for (const auto artistIdx : artists2check)
		{
			const auto& artist = Artists_ [artistIdx];
			const auto& albums = artist.Albums_;
			if (std::all_of (albums.begin (), albums.end (),
					[this] (const Collection::Album_ptr& album) { return IgnoredAlbums_.contains (album->ID_); }))
			{
				IgnoredArtists_ << artist.ID_;

				if (emitting)
				{
					const auto modelIdx = MakeArtistIndex (artistIdx);
					emit dataChanged (modelIdx, modelIdx);
				}
			}
		}
	}

	Util::DefaultScopeGuard LocalCollectionModel::AppendTracks (const AppendTracksByIds& info)
	{
		const auto [artistIdx, albumIdx] = FindIdx (Artists_, info.ArtistID_, info.AlbumID_);

		const auto curTracksCount = Artists_ [artistIdx].Albums_ [albumIdx]->Tracks_.size ();
		beginInsertRows (MakeAlbumIndex (artistIdx, albumIdx), curTracksCount, curTracksCount + info.NewTracksCount_ - 1);
		return EndInsertRowsGuard ();
	}

	Util::DefaultScopeGuard LocalCollectionModel::RemoveArtist (int artistId)
	{
		const auto idx = FindIdx (Artists_, artistId);
		if (idx == -1)
			return {};

		beginRemoveRows ({}, idx, idx);
		return EndRemoveRowsGuard ();
	}

	Util::DefaultScopeGuard LocalCollectionModel::RemoveAlbum (int artistId, int albumId)
	{
		const auto [artistIdx, albumIdx] = FindIdx (Artists_, artistId, albumId);
		if (artistIdx == -1 || albumIdx == -1)
			return {};

		beginRemoveRows (MakeArtistIndex (artistIdx), albumIdx, albumIdx);
		return EndRemoveRowsGuard ();
	}

	Util::DefaultScopeGuard LocalCollectionModel::RemoveTrack (int artistId, int albumId, int trackId)
	{
		const auto [artistIdx, albumIdx, trackIdx] = FindIdx (Artists_, artistId, albumId, trackId);
		if (artistIdx == -1 || albumIdx == -1 || trackIdx == -1)
			return {};

		beginRemoveRows (MakeAlbumIndex (artistIdx, albumIdx), trackIdx, trackIdx);
		return EndRemoveRowsGuard ();
	}

	void LocalCollectionModel::DataChanged (int artistId, int albumId)
	{
		const auto [artistIdx, albumIdx] = FindIdx (Artists_, artistId, albumId);

		const auto& idx = MakeAlbumIndex (artistIdx, albumIdx);
		emit dataChanged (idx, idx);
	}

	void LocalCollectionModel::UpdatePlayStats (int artistId, int albumId, int trackId)
	{
		ArtistTooltips_.remove (artistId);
		AlbumTooltips_.remove (albumId);
		TrackTooltips_.remove (trackId);
	}

	Util::DefaultScopeGuard LocalCollectionModel::EndInsertRowsGuard ()
	{
		return Util::MakeScopeGuard ([this] { endInsertRows (); });
	}

	Util::DefaultScopeGuard LocalCollectionModel::EndRemoveRowsGuard ()
	{
		return Util::MakeScopeGuard ([this] { endRemoveRows (); });
	}

	namespace
	{
		template<typename Stats>
		QString GetTooltip (int id, QCache<int, std::optional<Stats>>& cache,
				LocalCollectionStorage& storage, auto getter,
				auto fmtStats)
		{
			const auto mkTooltip = [&] (const std::optional<Stats>& stats)
			{
				return stats ? fmtStats (*stats) : LocalCollectionModel::tr ("Never has been played");
			};

			if (const auto cachedStatsPtr = cache.object (id))
				return mkTooltip (*cachedStatsPtr);

			const auto& maybeStats = (storage.*getter) (id);
			cache.insert (id, new std::optional<Stats> { maybeStats });
			return mkTooltip (maybeStats);
		}
	}

	QString LocalCollectionModel::GetArtistTooltip (int artistId) const
	{
		return GetTooltip (artistId, ArtistTooltips_, Storage_, &LocalCollectionStorage::GetArtistStats,
				[] (const Collection::ArtistStats& stats)
				{
					return tr ("Last playback: %1 (%2 from %3)")
							.arg (FormatDateTime (stats.LastPlayback_),
									Util::FormatName (stats.LastPlayedTrack_),
									Util::FormatName (stats.LastPlayedAlbum_));
				});
	}

	QString LocalCollectionModel::GetAlbumTooltip (int albumId) const
	{
		return GetTooltip (albumId, AlbumTooltips_, Storage_, &LocalCollectionStorage::GetAlbumStats,
				[] (const Collection::AlbumStats& stats)
				{
					return tr ("Last playback: %1 (%2)")
							.arg (FormatDateTime (stats.LastPlayback_),
									Util::FormatName (stats.LastPlayedTrack_));
				});
	}

	QString LocalCollectionModel::GetTrackTooltip (int trackId) const
	{
		return GetTooltip (trackId, TrackTooltips_, Storage_, &LocalCollectionStorage::GetTrackStats,
				[] (const Collection::TrackStats& stats)
				{
					return tr ("Last playback: %1")
									.arg (FormatDateTime (stats.LastPlay_))
							+ "\n"
							+ tr ("Played %n time(s) since %1", nullptr, stats.Playcount_)
									.arg (FormatDateTime (stats.Added_));
				});
	}
}
