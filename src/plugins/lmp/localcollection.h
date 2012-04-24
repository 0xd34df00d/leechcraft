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

		LocalCollectionStorage *Storage_;
		QStandardItemModel *CollectionModel_;
		QSortFilterProxyModel *Sorter_;

		Collection::Artists_t Artists_;
		QSet<QString> PresentPaths_;

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

		LocalCollection (QObject* = 0);

		QAbstractItemModel* GetCollectionModel () const;
		void Enqueue (const QModelIndex&, Player*);

		void Clear ();
		void Scan (const QString&);
	private:
		QStringList CollectPaths (const QModelIndex&);
		void AppendToModel (const Collection::Artists_t&);
	private slots:
		void handleLoadFinished ();
		void handleScanFinished ();
	signals:
		void scanStarted (int);
		void scanProgressChanged (int);
	};
}
}
