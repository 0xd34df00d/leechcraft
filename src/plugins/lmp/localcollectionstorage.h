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
#include <QSqlDatabase>
#include <QSqlQuery>
#include "mediainfo.h"
#include "interfaces/lmp/collectiontypes.h"

namespace LC::LMP
{
	struct RGData;

	class LocalCollectionStorage : public QObject
	{
		QHash<QString, int> PresentArtists_;
		QHash<QString, int> PresentAlbums_;

		QSqlDatabase DB_;

		QSqlQuery GetArtists_;
		QSqlQuery GetAlbums_;
		QSqlQuery GetAllTracks_;

		QSqlQuery AddArtist_;
		QSqlQuery AddAlbum_;
		QSqlQuery LinkArtistAlbum_;
		QSqlQuery AddTrack_;
		QSqlQuery AddGenre_;

		QSqlQuery IgnoreTrack_;
		QSqlQuery GetIgnoredTracks_;

		QSqlQuery RemoveTrack_;
		QSqlQuery RemoveAlbum_;
		QSqlQuery RemoveArtist_;

		QSqlQuery SetAlbumArt_;

		QSqlQuery GetTrackStats_;
		QSqlQuery SetTrackStats_;
		QSqlQuery UpdateTrackStats_;

		QSqlQuery GetFileIdMTime_;
		QSqlQuery GetFileMTime_;
		QSqlQuery SetFileMTime_;

		// 1 is loved, 2 is banned
		QSqlQuery GetLovedBanned_;
		QSqlQuery SetLovedBanned_;
		QSqlQuery RemoveLovedBanned_;

		QSqlQuery GetOutdatedRgData_;
		QSqlQuery GetTrackRgData_;
		QSqlQuery SetTrackRgData_;

		QSqlQuery AppendToPlayHistory_;
	public:
		struct LoadResult
		{
			Collection::Artists_t Artists_;

			QHash<QString, int> PresentArtists_;
			QHash<QString, int> PresentAlbums_;

			QSet<int> IgnoredTracks_;
		};

		LocalCollectionStorage (QObject* = nullptr);
		~LocalCollectionStorage ();

		void Clear ();
		Collection::Artists_t AddToCollection (const QList<MediaInfo>&);
		LoadResult Load ();
		void Load (const LoadResult&);

		QStringList GetTracksPaths ();

		void IgnoreTrack (int);
		QList<int> GetIgnoredTracks ();

		void RemoveTrack (int);
		void RemoveAlbum (int);
		void RemoveArtist (int);

		void SetAlbumArt (int, const QString&);

		Collection::TrackStats GetTrackStats (int);
		void SetTrackStats (const Collection::TrackStats&);
		void RecordTrackPlayed (int, const QDateTime&);

		QDateTime GetMTime (const QString&);
		void SetMTime (const QString&, const QDateTime&);

		void SetTrackLoved (int);
		void SetTrackBanned (int);
		void ClearTrackLovedBanned (int);
		QStringList GetLovedTracksPaths ();
		QStringList GetBannedTracksPaths ();

		QList<int> GetOutdatedRgAlbums ();
		void SetRgTrackInfo (int, const RGData&);
		RGData GetRgTrackInfo (const QString&);
	private:
		void MarkLovedBanned (int, int);
		QStringList GetLovedBanned (int);

		Collection::Artists_t GetAllArtists ();
		QHash<int, Collection::Album_ptr> GetAllAlbums ();

		void AddArtist (Collection::Artist&);
		void AddAlbum (const Collection::Artist&, Collection::Album&);
		void AddTrack (Collection::Track&, int, int);

		void AddToPresent (const Collection::Artist&);
		bool IsPresent (const Collection::Artist&, int&) const;
		void AddToPresent (const Collection::Artist&, const Collection::Album&);
		bool IsPresent (const Collection::Artist&, const Collection::Album&, int&) const;

		void PrepareQueries ();
		void CreateTables ();
	};
}
