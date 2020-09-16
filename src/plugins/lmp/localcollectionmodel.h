/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStandardItemModel>
#include <QHash>
#include <util/models/dndactionsmixin.h>
#include "interfaces/lmp/icollectionmodel.h"
#include "interfaces/lmp/collectiontypes.h"

namespace LC::LMP
{
	class LocalCollectionStorage;

	class LocalCollectionModel : public Util::DndActionsMixin<QStandardItemModel>
							   , public ICollectionModel
	{
		Q_OBJECT

		LocalCollectionStorage * const Storage_;

		QIcon ArtistIcon_ = QIcon::fromTheme ("view-media-artist");

		QHash<int, QStandardItem*> Artist2Item_;
		QHash<int, QHash<int, QStandardItem*>> Album2Item_;
		QHash<int, QStandardItem*> Track2Item_;
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
			AlbumYear,
			AlbumName,
			AlbumArt,
			TrackID,
			TrackNumber,
			TrackTitle,
			TrackPath,
			TrackGenres,
			TrackLength,
			IsTrackIgnored
		};

		LocalCollectionModel (LocalCollectionStorage*, QObject*);

		QStringList mimeTypes () const override;
		QMimeData* mimeData (const QModelIndexList&) const override;
		QVariant data (const QModelIndex& index, int role) const override;

		QList<QUrl> ToSourceUrls (const QList<QModelIndex>&) const override;

		void AddArtists (const Collection::Artists_t&);
		void Clear ();

		void IgnoreTrack (int);

		void RemoveTrack (int);
		void RemoveAlbum (int);
		void RemoveArtist (int);

		void SetAlbumArt (int, const QString&);
		QVariant GetTrackData (int trackId, Role) const;

		void UpdatePlayStats (int);
	};
}
