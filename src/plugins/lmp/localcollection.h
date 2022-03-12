/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <QSet>
#include <QFutureWatcher>
#include <QIcon>
#include "interfaces/lmp/collectiontypes.h"
#include "interfaces/lmp/ilocalcollection.h"
#include "mediainfo.h"
#include "localcollectionmodel.h"

class QModelIndex;
class QSortFilterProxyModel;

namespace LC::LMP
{
	class AlbumArtManager;
	class LocalCollectionStorage;
	class LocalCollectionWatcher;
	class Player;

	class LocalCollection : public QObject
						  , public ILocalCollection
	{
		Q_OBJECT
		Q_INTERFACES (LC::LMP::ILocalCollection)

		bool IsReady_ = false;

		QStringList RootPaths_;

		LocalCollectionStorage * const Storage_;
		LocalCollectionModel * const CollectionModel_;
		LocalCollectionWatcher * const FilesWatcher_;

		AlbumArtManager * const AlbumArtMgr_;

		Collection::Artists_t Artists_;

		QHash<QString, int> Path2Track_;

		QHash<int, int> Track2Album_;
		QHash<int, Collection::Album_ptr> AlbumID2Album_;
		QHash<int, int> AlbumID2ArtistID_;

		QFutureWatcher<MediaInfo> *Watcher_;
		QList<QSet<QString>> NewPathsQueue_;

		int UpdateNewArtists_ = 0;
		int UpdateNewAlbums_ = 0;
		int UpdateNewTracks_ = 0;
	public:
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

		explicit LocalCollection (QObject* = nullptr);

		bool IsReady () const;

		AlbumArtManager* GetAlbumArtManager () const;
		LocalCollectionStorage* GetStorage () const;
		QAbstractItemModel* GetCollectionModel () const;

		QVariant GetTrackData (int trackId, LocalCollectionModel::Role) const;

		void Clear ();

		void Scan (const QString&, bool root = true);
		void Unscan (const QString&);
		void Rescan ();

		DirStatus GetDirStatus (const QString&) const;
		QStringList GetDirs () const;

		int FindArtist (const QString&) const;

		int FindAlbum (const QString& artist, const QString& album) const;
		Collection::Album_ptr GetAlbum (int albumId) const;

		void SetAlbumArt (int, const QString&);

		int FindTrack (const QString& path) const;
		int GetTrackAlbumId (int trackId) const;
		Collection::Album_ptr GetTrackAlbum (int trackId) const;

		QStringList GetDynamicPlaylist (DynamicPlaylist) const;

		void AddTrackTo (int, StaticRating);

		Collection::TrackStats GetTrackStats (const QString&) const;

		QList<int> GetAlbumArtists (int) const;
		Collection::Artist GetArtist (int) const;
		Collection::Artists_t GetAllArtists () const override;

		void IgnoreTrack (const QString&);
		void RemoveTrack (const QString&);
		void RecordPlayedTrack (const QString&);
		void RecordPlayedTrack (int, const QDateTime&) override;
	private:
		void HandleExistingInfos (const QList<MediaInfo>&);
		void HandleNewArtists (Collection::Artists_t, const QSet<int>& = {});
		void RemoveAlbum (int);
		Collection::Artists_t::iterator RemoveArtist (Collection::Artists_t::iterator);

		void AddRootPaths (QStringList);
		void RemoveRootPaths (const QStringList&);

		void CheckRemovedFiles (const QSet<QString>& scanned, const QString& root);

		void InitiateScan (const QSet<QString>&);
		void RescanOnLoad ();
	private slots:
		void handleScanFinished ();
		void saveRootPaths ();
	signals:
		void scanStarted (int);
		void scanProgressChanged (int);
		void scanFinished ();

		void collectionReady ();

		void rootPathsChanged (const QStringList&);

		void gotNewArtists (const Collection::Artists_t&);
	};
}
