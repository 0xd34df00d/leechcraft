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
	class AlbumArtManager;
	class LocalCollectionStorage;
	class Player;
	class LocalCollectionWatcher;

	class LocalCollection : public QObject
	{
		Q_OBJECT

		bool IsReady_;

		QStringList RootPaths_;

		QIcon ArtistIcon_;
		LocalCollectionStorage *Storage_;
		QStandardItemModel *CollectionModel_;
		QSortFilterProxyModel *Sorter_;
		LocalCollectionWatcher *FilesWatcher_;

		AlbumArtManager *AlbumArtMgr_;

		Collection::Artists_t Artists_;

		QSet<QString> PresentPaths_;
		QHash<QString, int> Path2Track_;
		QHash<int, QString> Track2Path_;

		QHash<int, int> Track2Album_;
		QHash<int, Collection::Album_ptr> AlbumID2Album_;

		QHash<int, QStandardItem*> Artist2Item_;
		QHash<int, QStandardItem*> Album2Item_;
		QHash<int, QStandardItem*> Track2Item_;

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

		enum class DirStatus
		{
			RootPath,
			SubPath,
			None
		};

		LocalCollection (QObject* = 0);
		void FinalizeInit ();

		bool IsReady () const;

		QAbstractItemModel* GetCollectionModel () const;
		void Enqueue (const QModelIndex&, Player*);
		void Clear ();

		void Scan (const QString&, bool root = true);
		void Unscan (const QString&);
		void Rescan ();

		DirStatus GetDirStatus (const QString&) const;
		QStringList GetDirs () const;

		int FindAlbum (const QString&, const QString&) const;
		void SetAlbumArt (int, const QString&);

		int FindTrack (const QString&) const;
		Collection::Album_ptr GetTrackAlbum (int) const;

		QList<int> GetDynamicPlaylist (DynamicPlaylist) const;
		QStringList TrackList2PathList (const QList<int>&) const;

		Collection::TrackStats GetTrackStats (const QString&);
		QStringList CollectPaths (const QModelIndex&);
	private:
		void HandleNewArtists (const Collection::Artists_t&);
		void RemoveTrack (const QString&);
		void RemoveAlbum (int);
		Collection::Artists_t::iterator RemoveArtist (Collection::Artists_t::iterator);

		void AddRootPaths (const QStringList&);
		void RemoveRootPaths (const QStringList&);
	public slots:
		void recordPlayedTrack (const QString&);
	private slots:
		void handleLoadFinished ();
		void handleIterateFinished ();
		void handleScanFinished ();
		void saveRootPaths ();
	signals:
		void scanStarted (int);
		void scanProgressChanged (int);

		void collectionReady ();

		void rootPathsChanged (const QStringList&);
	};
}
}
