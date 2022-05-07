/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QAbstractItemModel>
#include <QIcon>
#include <QHash>
#include <QCache>
#include <util/models/dndactionsmixin.h>
#include <util/sll/util.h>
#include "interfaces/lmp/icollectionmodel.h"
#include "interfaces/lmp/collectiontypes.h"

namespace LC::LMP
{
	class LocalCollection;
	class LocalCollectionStorage;

	class LocalCollectionModel : public Util::DndActionsMixin<QAbstractItemModel>
							   , public ICollectionModel
	{
		Q_OBJECT

		const Collection::Artists_t& Artists_;
		LocalCollectionStorage& Storage_;
		LocalCollection& Collection_;

		QIcon ArtistIcon_ = QIcon::fromTheme (QStringLiteral ("view-media-artist"));

		QSet<int> IgnoredTracks_;
		QSet<int> IgnoredAlbums_;
		QSet<int> IgnoredArtists_;

		template<typename T>
		using Cache_t = QCache<int, std::optional<T>>;
		mutable Cache_t<Collection::ArtistStats> ArtistTooltips_;
		mutable Cache_t<Collection::AlbumStats> AlbumTooltips_;
		mutable Cache_t<Collection::TrackStats> TrackTooltips_;
	public:
		enum NodeType
		{
			Artist,
			Album,
			Track
		};

		enum Role
		{
			Node = Qt::UserRole + 1,
			ArtistName,
			AlbumID,
			AlbumYear,
			AlbumName,
			AlbumArt,
			TrackID,
			TrackNumber,
			TrackTitle,
			TrackPath,
			TrackGenres,
			TrackLength,
			IsIgnored
		};

		LocalCollectionModel (const Collection::Artists_t&, LocalCollectionStorage&, LocalCollection&);

		QStringList mimeTypes () const override;
		QMimeData* mimeData (const QModelIndexList&) const override;

		int columnCount (const QModelIndex&) const override;
		QVariant data (const QModelIndex&, int) const override;
		Qt::ItemFlags flags (const QModelIndex&) const override;
		QModelIndex index (int, int, const QModelIndex&) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex&) const override;

		QList<QUrl> ToSourceUrls (const QList<QModelIndex>&) const override;

		void IgnoreTracks (const QSet<int>&, bool emitting = true);

		[[nodiscard]] Util::DefaultScopeGuard ResetArtists ();
		[[nodiscard]] Util::DefaultScopeGuard InsertArtist (int idx);
		[[nodiscard]] Util::DefaultScopeGuard AppendAlbums (int artistIdx, int newAlbumsCount);

		struct AppendTracksByIds
		{
			int ArtistID_;
			int AlbumID_;
			int NewTracksCount_;
		};
		[[nodiscard]] Util::DefaultScopeGuard AppendTracks (const AppendTracksByIds&);

		[[nodiscard]] Util::DefaultScopeGuard RemoveArtist (int artistId);
		[[nodiscard]] Util::DefaultScopeGuard RemoveAlbum (int artistId, int albumId);
		[[nodiscard]] Util::DefaultScopeGuard RemoveTrack (int artistId, int albumId, int trackId);

		void DataChanged (int artistId, int albumId);

		void UpdatePlayStats (int, int, int);
	private:
		QModelIndex MakeArtistIndex (quintptr artistIdx) const;
		QModelIndex MakeAlbumIndex (quintptr artistIdx, quintptr albumIdx) const;
		QModelIndex MakeTrackIndex (quintptr artistIdx, quintptr albumIdx, quintptr trackIdx) const;

		Util::DefaultScopeGuard EndInsertRowsGuard ();
		Util::DefaultScopeGuard EndRemoveRowsGuard ();

		QString GetArtistTooltip (int artistId) const;
		QString GetAlbumTooltip (int albumId) const;
		QString GetTrackTooltip (int trackId) const;
	};
}
