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
#include "interfaces/lmp/collectiontypes.h"
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
		QHash<int, int> AlbumID2ArtistID_;

		QHash<int, QStandardItem*> Artist2Item_;
		QHash<int, QStandardItem*> Album2Item_;
		QHash<int, QStandardItem*> Track2Item_;

		QFutureWatcher<MediaInfo> *Watcher_;
		QList<QSet<QString>> NewPathsQueue_;

		int UpdateNewArtists_;
		int UpdateNewAlbums_;
		int UpdateNewTracks_;
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
			Random50,
			LovedTracks,
			BannedTracks
		};

		enum class DirStatus
		{
			RootPath,
			SubPath,
			None
		};

		enum class StaticRating
		{
			Loved,
			Banned
		};

		LocalCollection (QObject* = 0);
		void FinalizeInit ();

		bool IsReady () const;

		QAbstractItemModel* GetCollectionModel () const;
		void Enqueue (const QModelIndex&, Player*);
		void Enqueue (const QList<QModelIndex>&, Player*);
		void Clear ();

		void Scan (const QString&, bool root = true);
		void Unscan (const QString&);
		void Rescan ();

		DirStatus GetDirStatus (const QString&) const;
		QStringList GetDirs () const;

		int FindArtist (const QString&) const;

		int FindAlbum (const QString& artist, const QString& album) const;
		void SetAlbumArt (int, const QString&);
		Collection::Album_ptr GetAlbum (int albumId) const;

		int FindTrack (const QString&) const;
		Collection::Album_ptr GetTrackAlbum (int trackId) const;
		QVariant GetTrackData (int trackId, Role) const;

		QList<int> GetDynamicPlaylist (DynamicPlaylist) const;
		QStringList TrackList2PathList (const QList<int>&) const;

		void AddTrackTo (int, StaticRating);

		Collection::TrackStats GetTrackStats (const QString&) const;

		QList<int> GetAlbumArtists (int) const;
		Collection::Artist GetArtist (int) const;
		Collection::Artists_t GetAllArtists () const;

		void RemoveTrack (const QString&);
	private:
		void HandleExistingInfos (const QList<MediaInfo>&);
		void HandleNewArtists (const Collection::Artists_t&);
		void RemoveAlbum (int);
		Collection::Artists_t::iterator RemoveArtist (Collection::Artists_t::iterator);

		void AddRootPaths (QStringList);
		void RemoveRootPaths (const QStringList&);

		void CheckRemovedFiles (const QSet<QString>& scanned, const QString& root);

		void InitiateScan (const QSet<QString>&);
	public slots:
		void recordPlayedTrack (const QString&);
	private slots:
		void rescanOnLoad ();
		void handleLoadFinished ();
		void handleIterateFinished ();
		void handleScanFinished ();
		void saveRootPaths ();
	signals:
		void scanStarted (int);
		void scanProgressChanged (int);
		void scanFinished ();

		void collectionReady ();

		void rootPathsChanged (const QStringList&);
	};
}
}
