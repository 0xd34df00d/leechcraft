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

#pragma once

#include <QObject>
#include <QHash>
#include <QSet>
#include <QFutureWatcher>
#include <QIcon>
#include "collectiontypes.h"
#include "mediainfo.h"

class QStandardItemModel;
class QStandardItem;
class QAbstractItemModel;
class QModelIndex;
class QSortFilterProxyModel;

namespace LeechCraft
{
namespace LMP
{
	class LocalCollectionStorage;
	class Player;

	class LocalCollection : public QObject
	{
		Q_OBJECT

		QIcon ArtistIcon_;
		LocalCollectionStorage *Storage_;
		QStandardItemModel *CollectionModel_;
		QSortFilterProxyModel *Sorter_;

		Collection::Artists_t Artists_;
		QSet<QString> PresentPaths_;
		QHash<QString, int> Path2Track_;
		QHash<int, QString> Track2Path_;

		QHash<int, QStandardItem*> Artist2Item_;
		QHash<int, QStandardItem*> Album2Item_;

		QFutureWatcher<MediaInfo> *Watcher_;
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
			TrackNumber,
			TrackTitle,
			TrackPath
		};
		enum class DynamicPlaylist
		{
			Random50
		};

		LocalCollection (QObject* = 0);

		void FinalizeInit ();

		QAbstractItemModel* GetCollectionModel () const;
		void Enqueue (const QModelIndex&, Player*);

		void Clear ();
		void Scan (const QString&);

		QList<int> GetDynamicPlaylist (DynamicPlaylist) const;
		QStringList TrackList2PathList (const QList<int>&) const;

		Collection::TrackStats GetTrackStats (const QString&);
	private:
		QStringList CollectPaths (const QModelIndex&);
		void HandleNewArtists (const Collection::Artists_t&);
	public slots:
		void recordPlayedTrack (const QString&);
	private slots:
		void handleLoadFinished ();
		void handleScanFinished ();
	signals:
		void scanStarted (int);
		void scanProgressChanged (int);
	};
}
}
