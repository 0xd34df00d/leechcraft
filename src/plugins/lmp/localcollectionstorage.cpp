/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "localcollectionstorage.h"
#include <stdexcept>
#include <QSqlError>
#include <QSqlQuery>
#include <QFileInfo>
#include <QThread>
#include <QDir>
#include <util/util.h>
#include <util/db/dblock.h>
#include <util/db/util.h>
#include <util/sll/containerconversions.h>
#include <util/sys/paths.h>
#include "util.h"
#include "engine/rgfilter.h"
#include "xmlsettingsmanager.h"

namespace LC::LMP
{
	LocalCollectionStorage::LocalCollectionStorage (QObject *parent)
	: QObject (parent)
	, DB_ (QSqlDatabase::addDatabase ("QSQLITE",
			Util::GenConnectionName ("org.LMP.LocalCollection")))
	{
		DB_.setDatabaseName (Util::CreateIfNotExists ("lmp").filePath ("localcollection.db"));

		if (!DB_.open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open the database";
			Util::DBLock::DumpError (DB_.lastError ());
			throw std::runtime_error ("unable to open Azoth history database");
		}

		{
			QSqlQuery query (DB_);
			query.exec ("PRAGMA foreign_keys = ON;");
			query.exec ("PRAGMA synchronous = OFF;");
		}

		try
		{
			CreateTables ();
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
		}

		PrepareQueries ();
	}

	LocalCollectionStorage::~LocalCollectionStorage ()
	{
		DB_.close ();
	}

	void LocalCollectionStorage::Clear ()
	{
		Util::DBLock lock (DB_);
		lock.Init ();
		QSqlQuery query (DB_);
		if (!query.exec ("DELETE FROM artists;") ||
			!query.exec ("DELETE FROM albums;"))
		{
			Util::DBLock::DumpError (query);
			throw std::runtime_error ("unable to clear database");
		}
		lock.Good ();

		PresentAlbums_.clear ();
		PresentArtists_.clear ();
	}

	Collection::Artists_t LocalCollectionStorage::AddToCollection (const QList<MediaInfo>& infos)
	{
		QMap<int, Collection::Artist> artists;

		Util::DBLock lock (DB_);

		lock.Init ();
		for (const auto& info : infos)
		{
			Collection::Artist artist
			{
				0,
				info.Artist_,
				{}
			};
			if (!IsPresent (artist, artist.ID_))
				AddArtist (artist);
			if (!artists.contains (artist.ID_))
				artists [artist.ID_] = artist;

			Collection::Album album
			{
				0,
				info.Album_,
				info.Year_,
				{},
				{}
			};
			if (!IsPresent (artist, album, album.ID_))
			{
				album.CoverPath_ = FindAlbumArtPath (info.LocalPath_);
				AddAlbum (artist, album);
				artists [artist.ID_].Albums_ << std::make_shared<Collection::Album> (album);
			}

			Collection::Track track
			{
				0,
				info.TrackNumber_,
				info.Title_,
				info.Length_,
				info.Genres_,
				info.LocalPath_
			};
			AddTrack (track, artist.ID_, album.ID_);

			for (auto& trackAlbum : artists [artist.ID_].Albums_)
				if (trackAlbum->ID_ == album.ID_)
				{
					trackAlbum->Tracks_ << track;
					break;
				}

			SetMTime (info.LocalPath_, QFileInfo { info.LocalPath_ }.lastModified ());
		}
		lock.Good ();

		return artists.values ();
	}

	LocalCollectionStorage::LoadResult LocalCollectionStorage::Load ()
	{
		qDebug () << "begin";
		Collection::Artists_t artists = GetAllArtists ();
		qDebug () << "artists";
		const auto& albums = GetAllAlbums ();
		qDebug () << "albums";

		QSqlQuery links (DB_);
		links.exec ("SELECT ArtistID, AlbumID FROM artists2albums;");
		QHash<int, QList<int>> artist2albums;
		while (links.next ())
			artist2albums [links.value (0).toInt ()] << links.value (1).toInt ();
		links.finish ();

		qDebug () << "filled";
		for (auto& artist : artists)
		{
			AddToPresent (artist);
			for (auto aid : artist2albums [artist.ID_])
			{
				auto album = albums [aid];
				if (!album)
				{
					qWarning () << Q_FUNC_INFO
							<< "no album for"
							<< artist.ID_
							<< aid;
					continue;
				}
				artist.Albums_ << album;
				AddToPresent (artist, *album);
			}
		}

		LoadResult result =
		{
			artists,
			PresentArtists_,
			PresentAlbums_,
			Util::AsSet (GetIgnoredTracks ())
		};
		qDebug () << "end";

		return result;
	}

	void LocalCollectionStorage::Load (const LoadResult& result)
	{
		PresentAlbums_ = result.PresentAlbums_;
		PresentArtists_ = result.PresentArtists_;
	}

	QStringList LocalCollectionStorage::GetTracksPaths ()
	{
		if (!GetAllTracks_.exec ())
		{
			Util::DBLock::DumpError (GetAllTracks_);
			throw std::runtime_error ("cannot get all tracks");
		}

		QStringList result;
		result.reserve (GetAllTracks_.size ());
		while (GetAllTracks_.next ())
			result << GetAllTracks_.value (1).toString ();

		GetAllTracks_.finish ();

		return result;
	}

	void LocalCollectionStorage::IgnoreTrack (int id)
	{
		IgnoreTrack_.bindValue (":track_id", id);
		if (!IgnoreTrack_.exec ())
		{
			Util::DBLock::DumpError (IgnoreTrack_);
			throw std::runtime_error ("cannot ignore track");
		}
	}

	QList<int> LocalCollectionStorage::GetIgnoredTracks ()
	{
		if (!GetIgnoredTracks_.exec ())
		{
			Util::DBLock::DumpError (GetIgnoredTracks_);
			throw std::runtime_error ("cannot get ignored tracks track");
		}

		QList<int> result;
		while (GetIgnoredTracks_.next ())
			result << GetIgnoredTracks_.value (0).toInt ();

		GetIgnoredTracks_.finish ();

		return result;
	}

	void LocalCollectionStorage::RemoveTrack (int id)
	{
		RemoveTrack_.bindValue (":track_id", id);
		if (!RemoveTrack_.exec ())
		{
			Util::DBLock::DumpError (RemoveTrack_);
			throw std::runtime_error ("cannot remove track");
		}
	}

	void LocalCollectionStorage::RemoveAlbum (int id)
	{
		RemoveAlbum_.bindValue (":album_id", id);
		if (!RemoveAlbum_.exec ())
		{
			Util::DBLock::DumpError (RemoveAlbum_);
			throw std::runtime_error ("cannot remove album");
		}

		PresentAlbums_.remove (PresentAlbums_.key (id));
	}

	void LocalCollectionStorage::RemoveArtist (int id)
	{
		RemoveArtist_.bindValue (":artist_id", id);
		if (!RemoveArtist_.exec ())
		{
			Util::DBLock::DumpError (RemoveArtist_);
			throw std::runtime_error ("cannot remove artist");
		}

		PresentArtists_.remove (PresentArtists_.key (id));
	}

	void LocalCollectionStorage::SetAlbumArt (int id, const QString& path)
	{
		SetAlbumArt_.bindValue (":album_id", id);
		SetAlbumArt_.bindValue (":cover_path", path);
		if (!SetAlbumArt_.exec ())
		{
			Util::DBLock::DumpError (SetAlbumArt_);
			throw std::runtime_error ("cannot update album art");
		}
	}

	std::optional<Collection::TrackStats> LocalCollectionStorage::GetTrackStats (int trackId)
	{
		GetTrackStats_.bindValue (":track_id", trackId);
		if (!GetTrackStats_.exec ())
		{
			Util::DBLock::DumpError (GetTrackStats_);
			throw std::runtime_error ("cannot fetch track statistics");
		}

		if (!GetTrackStats_.next ())
			return {};

		Collection::TrackStats result
		{
			GetTrackStats_.value (0).toInt (),
			GetTrackStats_.value (1).toDateTime (),
			GetTrackStats_.value (2).toDateTime (),
			GetTrackStats_.value (3).toInt (),
			GetTrackStats_.value (4).toInt ()
		};
		GetTrackStats_.finish ();

		return result;
	}

	void LocalCollectionStorage::SetTrackStats (int trackId, const Collection::TrackStats& stats)
	{
		SetTrackStats_.bindValue (":track_id", trackId);
		SetTrackStats_.bindValue (":playcount", stats.Playcount_);
		SetTrackStats_.bindValue (":added", stats.Added_);
		SetTrackStats_.bindValue (":last_play", stats.LastPlay_);

		if (!SetTrackStats_.exec ())
		{
			Util::DBLock::DumpError (SetTrackStats_);
			throw std::runtime_error ("cannot set track statistics");
		}
	}

	void LocalCollectionStorage::RecordTrackPlayed (int trackId, const QDateTime& date)
	{
		AppendToPlayHistory_.bindValue (":track_id", trackId);
		AppendToPlayHistory_.bindValue (":date", date);

		if (!AppendToPlayHistory_.exec ())
			Util::DBLock::DumpError (AppendToPlayHistory_);

		UpdateTrackStats_.bindValue (":track_id", trackId);
		UpdateTrackStats_.bindValue (":track_id_pc", trackId);
		UpdateTrackStats_.bindValue (":track_id_add", trackId);
		UpdateTrackStats_.bindValue (":track_id_lp", trackId);
		UpdateTrackStats_.bindValue (":add_date", date);
		UpdateTrackStats_.bindValue (":play_date", date);

		if (!UpdateTrackStats_.exec ())
		{
			Util::DBLock::DumpError (UpdateTrackStats_);
			throw std::runtime_error ("cannot update track statistics");
		}
	}

	std::optional<Collection::AlbumStats> LocalCollectionStorage::GetAlbumStats (int albumId)
	{
		GetAlbumStats_.bindValue (":album_id", albumId);
		Util::DBLock::Execute (GetAlbumStats_);

		if (!GetAlbumStats_.next ())
			return {};

		Collection::AlbumStats stats
		{
			.LastPlayback_ = GetAlbumStats_.value (0).toDateTime (),
			.LastPlayedTrack_ = GetAlbumStats_.value (1).toString (),
		};
		GetAlbumStats_.finish ();
		return stats;
	}

	std::optional<Collection::ArtistStats> LocalCollectionStorage::GetArtistStats (int artistId)
	{
		GetArtistStats_.bindValue (":artist_id", artistId);
		Util::DBLock::Execute (GetArtistStats_);

		if (!GetArtistStats_.next ())
			return {};

		Collection::ArtistStats stats
		{
			.LastPlayback_ = GetArtistStats_.value (0).toDateTime (),
			.LastPlayedAlbum_ = GetArtistStats_.value (1).toString (),
			.LastPlayedTrack_ = GetArtistStats_.value (2).toString (),
		};
		GetArtistStats_.finish ();
		return stats;
	}

	QDateTime LocalCollectionStorage::GetMTime (const QString& filepath)
	{
		GetFileMTime_.bindValue (":filepath", filepath);
		if (!GetFileMTime_.exec ())
		{
			Util::DBLock::DumpError (GetFileMTime_);
			throw std::runtime_error ("cannot get file mtime");
		}

		const auto& result = GetFileMTime_.next () ?
				GetFileMTime_.value (0).toDateTime () :
				QDateTime ();
		GetFileMTime_.finish ();
		return result;
	}

	void LocalCollectionStorage::SetMTime (const QString& filepath, const QDateTime& mtime)
	{
		SetFileMTime_.bindValue (":filepath", filepath);
		SetFileMTime_.bindValue (":mtime", mtime);
		if (!SetFileMTime_.exec ())
		{
			Util::DBLock::DumpError (SetFileMTime_);
			throw std::runtime_error ("cannot set file mtime");
		}
	}

	const int LovedStateID = 1;
	const int BannedStateID = 2;

	void LocalCollectionStorage::SetTrackLoved (int trackId)
	{
		MarkLovedBanned (trackId, LovedStateID);
	}

	void LocalCollectionStorage::SetTrackBanned (int trackId)
	{
		MarkLovedBanned (trackId, BannedStateID);
	}

	void LocalCollectionStorage::ClearTrackLovedBanned (int trackId)
	{
		RemoveLovedBanned_.bindValue (":track_id", trackId);
		if (!RemoveLovedBanned_.exec ())
		{
			Util::DBLock::DumpError (RemoveLovedBanned_);
			throw std::runtime_error ("cannot remove track from loved/banned");
		}
	}

	QStringList LocalCollectionStorage::GetLovedTracksPaths ()
	{
		return GetLovedBanned (LovedStateID);
	}

	QStringList LocalCollectionStorage::GetBannedTracksPaths ()
	{
		return GetLovedBanned (BannedStateID);
	}

	QList<int> LocalCollectionStorage::GetOutdatedRgAlbums ()
	{
		if (!GetOutdatedRgData_.exec ())
		{
			Util::DBLock::DumpError (GetOutdatedRgData_);
			throw std::runtime_error ("cannot fetch outdated track RG data");
		}

		QList<int> result;
		while (GetOutdatedRgData_.next ())
			result << GetOutdatedRgData_.value (0).toInt ();
		return result;
	}

	void LocalCollectionStorage::SetRgTrackInfo (int trackId, const RGData& data)
	{
		GetFileIdMTime_.bindValue (":track_id", trackId);
		if (!GetFileIdMTime_.exec ())
		{
			Util::DBLock::DumpError (GetFileIdMTime_);
			throw std::runtime_error ("cannot get file mtime");
		}

		const auto& mtime = GetFileIdMTime_.next () ?
				GetFileIdMTime_.value (0).toDateTime () :
				QDateTime ();
		GetFileIdMTime_.finish ();

		SetTrackRgData_.bindValue (":track_id", trackId);
		SetTrackRgData_.bindValue (":mtime", mtime);
		SetTrackRgData_.bindValue (":track_gain", data.TrackGain_);
		SetTrackRgData_.bindValue (":track_peak", data.TrackPeak_);
		SetTrackRgData_.bindValue (":album_gain", data.AlbumGain_);
		SetTrackRgData_.bindValue (":album_peak", data.AlbumPeak_);

		if (!SetTrackRgData_.exec ())
		{
			Util::DBLock::DumpError (SetTrackRgData_);
			throw std::runtime_error ("cannot set track RG data");
		}
	}

	RGData LocalCollectionStorage::GetRgTrackInfo (const QString& filepath)
	{
		GetTrackRgData_.bindValue (":filepath", filepath);

		if (!GetTrackRgData_.exec ())
		{
			Util::DBLock::DumpError (GetTrackRgData_);
			throw std::runtime_error ("cannot get track RG data");
		}

		if (!GetTrackRgData_.next ())
			return {};

		const RGData data
		{
			GetTrackRgData_.value (0).toDouble (),
			GetTrackRgData_.value (1).toDouble (),
			GetTrackRgData_.value (2).toDouble (),
			GetTrackRgData_.value (3).toDouble ()
		};

		GetTrackRgData_.finish ();

		return data;
	}

	void LocalCollectionStorage::MarkLovedBanned (int trackId, int state)
	{
		SetLovedBanned_.bindValue (":track_id", trackId);
		SetLovedBanned_.bindValue (":state", state);
		if (!SetLovedBanned_.exec ())
		{
			Util::DBLock::DumpError (SetLovedBanned_);
			throw std::runtime_error ("cannot mark track as loved/banned");
		}
	}

	QStringList LocalCollectionStorage::GetLovedBanned (int state)
	{
		GetLovedBanned_.bindValue (":state", state);
		if (!GetLovedBanned_.exec ())
		{
			Util::DBLock::DumpError (GetLovedBanned_);
			throw std::runtime_error ("cannot get loved/banned tracks");
		}

		QStringList result;
		while (GetLovedBanned_.next ())
			result << GetLovedBanned_.value (0).toString ();
		GetLovedBanned_.finish ();
		return result;
	}

	Collection::Artists_t LocalCollectionStorage::GetAllArtists ()
	{
		Collection::Artists_t artists;

		if (!GetArtists_.exec ())
		{
			Util::DBLock::DumpError (GetArtists_);
			throw std::runtime_error ("cannot fetch artists");
		}
		while (GetArtists_.next ())
		{
			Collection::Artist a =
			{
				GetArtists_.value (0).toInt (),
				GetArtists_.value (1).toString (),
				QList<Collection::Album_ptr> ()
			};

			artists << a;
		}
		GetArtists_.finish ();

		return artists;
	}

	QHash<int, Collection::Album_ptr> LocalCollectionStorage::GetAllAlbums ()
	{
		QHash<int, Collection::Album_ptr> newAlbums;

		QSqlQuery getter (DB_);
		QHash<int, QStringList> trackGenres;
		if (!getter.exec ("SELECT TrackId, Name FROM genres;"))
		{
			Util::DBLock::DumpError (getter);
			throw std::runtime_error ("cannot fetch genres");
		}

		while (getter.next ())
			trackGenres [getter.value (0).toInt ()] << getter.value (1).toString ();

		if (!getter.exec (R"(SELECT
								albums.Id,
								albums.Name,
								albums.Year,
								albums.CoverPath,
								tracks.Id,
								tracks.TrackNumber,
								tracks.Name,
								tracks.Length,
								tracks.Path
								FROM tracks INNER JOIN albums ON tracks.AlbumID = albums.Id;
				)"))
		{
			Util::DBLock::DumpError (getter);
			throw std::runtime_error ("cannot fetch albums");
		}

		while (getter.next ())
		{
			const int albumID = getter.value (0).toInt ();
			auto albumPos = newAlbums.find (albumID);
			if (albumPos == newAlbums.end ())
			{
				const Collection::Album a
				{
					albumID,
					getter.value (1).toString (),
					getter.value (2).toInt (),
					getter.value (3).toString (),
					{}
				};
				albumPos = newAlbums.insert (albumID, std::make_shared<Collection::Album> (a));
			}

			auto albumPtr = *albumPos;
			auto& tracks = albumPtr->Tracks_;

			const int trackId = getter.value (4).toInt ();
			Collection::Track t
			{
				trackId,
				getter.value (5).toInt (),
				getter.value (6).toString (),
				getter.value (7).toInt (),
				trackGenres.value (trackId),
				getter.value (8).toString ()
			};

			tracks << t;
		}
		getter.finish ();

		return newAlbums;
	}

	void LocalCollectionStorage::AddArtist (Collection::Artist& artist)
	{
		AddArtist_.bindValue (":name", artist.Name_);
		if (!AddArtist_.exec ())
		{
			Util::DBLock::DumpError (AddArtist_);
			throw std::runtime_error ("cannot add artist");
		}
		artist.ID_ = AddArtist_.lastInsertId ().toInt ();

		AddToPresent (artist);
	}

	void LocalCollectionStorage::AddAlbum (const Collection::Artist& artist, Collection::Album& album)
	{
		AddAlbum_.bindValue (":name", album.Name_);
		AddAlbum_.bindValue (":year", album.Year_);
		AddAlbum_.bindValue (":cover_path", album.CoverPath_);
		if (!AddAlbum_.exec ())
		{
			Util::DBLock::DumpError (AddAlbum_);
			throw std::runtime_error ("cannot add album");
		}
		const int id = AddAlbum_.lastInsertId ().toInt ();
		album.ID_ = id;

		LinkArtistAlbum_.bindValue (":artist_id", artist.ID_);
		LinkArtistAlbum_.bindValue (":album_id", album.ID_);
		if (!LinkArtistAlbum_.exec ())
		{
			Util::DBLock::DumpError (LinkArtistAlbum_);
			throw std::runtime_error ("cannot link artist/album");
		}

		AddToPresent (artist, album);
	}

	void LocalCollectionStorage::AddTrack (Collection::Track& track, int artistId, int albumID)
	{
		AddTrack_.bindValue (":artist_id", artistId);
		AddTrack_.bindValue (":album_id", albumID);
		AddTrack_.bindValue (":path", track.FilePath_);
		AddTrack_.bindValue (":name", track.Name_);
		AddTrack_.bindValue (":track_number", track.Number_);
		AddTrack_.bindValue (":length", track.Length_);
		if (!AddTrack_.exec ())
		{
			Util::DBLock::DumpError (AddTrack_);
			throw std::runtime_error ("unable to add track");
		}
		const int id = AddTrack_.lastInsertId ().toInt ();
		track.ID_ = id;

		for (const auto& genre : track.Genres_)
		{
			AddGenre_.bindValue (":track_id", id);
			AddGenre_.bindValue (":name", genre);
			if (!AddGenre_.exec ())
			{
				Util::DBLock::DumpError (AddGenre_);
				throw std::runtime_error ("unable to add genre");
			}
		}
	}

	void LocalCollectionStorage::AddToPresent (const Collection::Artist& artist)
	{
		PresentArtists_ [artist.Name_] = artist.ID_;
	}

	bool LocalCollectionStorage::IsPresent (const Collection::Artist& artist, int& id) const
	{
		if (PresentArtists_.contains (artist.Name_))
		{
			id = PresentArtists_ [artist.Name_];
			return true;
		}
		else
			return false;
	}

	void LocalCollectionStorage::AddToPresent (const Collection::Artist& artist, const Collection::Album& album)
	{
		PresentAlbums_ [artist.Name_ + '_' + album.Name_ + '_' + QString::number (album.Year_)] = album.ID_;
	}

	bool LocalCollectionStorage::IsPresent (const Collection::Artist& artist, const Collection::Album& album, int& id) const
	{
		const QString& str = artist.Name_ + '_' + album.Name_ + '_' + QString::number (album.Year_);
		if (PresentAlbums_.contains (str))
		{
			id = PresentAlbums_ [str];
			return true;
		}
		else
			return false;
	}

	void LocalCollectionStorage::PrepareQueries ()
	{
		GetArtists_ = QSqlQuery (DB_);
		GetArtists_.prepare ("SELECT Id, Name FROM artists;");

		GetAlbums_ = QSqlQuery (DB_);
		GetAlbums_.prepare ("SELECT Id, Name, Year, CoverPath FROM albums;");

		GetAllTracks_ = QSqlQuery (DB_);
		GetAllTracks_.prepare ("SELECT Id, Path FROM tracks;");

		AddArtist_ = QSqlQuery (DB_);
		AddArtist_.prepare ("INSERT INTO artists (Name) VALUES (:name);");

		AddAlbum_ = QSqlQuery (DB_);
		AddAlbum_.prepare ("INSERT INTO albums (Name, Year, CoverPath) VALUES (:name, :year, :cover_path);");

		LinkArtistAlbum_ = QSqlQuery (DB_);
		LinkArtistAlbum_.prepare ("INSERT INTO artists2albums (ArtistID, AlbumID) VALUES (:artist_id, :album_id);");

		AddTrack_ = QSqlQuery (DB_);
		AddTrack_.prepare (R"(
				INSERT INTO tracks (ArtistID, AlbumID, Path, Name, TrackNumber, Length)
				VALUES (:artist_id, :album_id, :path, :name, :track_number, :length);
				)");

		AddGenre_ = QSqlQuery (DB_);
		AddGenre_.prepare ("INSERT INTO genres (TrackId, Name) VALUES (:track_id, :name);");

		IgnoreTrack_ = QSqlQuery (DB_);
		IgnoreTrack_.prepare ("INSERT INTO ignored_tracks (TrackId) VALUES (:track_id);");

		GetIgnoredTracks_ = QSqlQuery (DB_);
		GetIgnoredTracks_.prepare ("SELECT TrackId FROM ignored_tracks;");

		RemoveTrack_ = QSqlQuery (DB_);
		RemoveTrack_.prepare ("DELETE FROM tracks WHERE Id = :track_id;");

		RemoveAlbum_ = QSqlQuery (DB_);
		RemoveAlbum_.prepare ("DELETE FROM albums WHERE Id = :album_id;");

		RemoveArtist_ = QSqlQuery (DB_);
		RemoveArtist_.prepare ("DELETE FROM artists WHERE Id = :artist_id;");

		SetAlbumArt_ = QSqlQuery (DB_);
		SetAlbumArt_.prepare ("UPDATE albums SET CoverPath = :cover_path WHERE Id = :album_id");

		GetTrackStats_ = QSqlQuery (DB_);
		GetTrackStats_.prepare ("SELECT Playcount, Added, LastPlay, Score, Rating FROM statistics WHERE TrackId = :track_id;");

		SetTrackStats_ = QSqlQuery (DB_);
		SetTrackStats_.prepare (R"(
				INSERT OR REPLACE INTO statistics (TrackId, Playcount, Added, LastPlay)
				VALUES (:track_id, :playcount, :added, :last_play);
				)");

		UpdateTrackStats_ = QSqlQuery (DB_);
		UpdateTrackStats_.prepare (R"(
				INSERT OR REPLACE INTO statistics (TrackId, Playcount, Added, LastPlay)
				VALUES (:track_id,
						coalesce ((SELECT Playcount FROM statistics WHERE TrackId = :track_id_pc), 0) + 1,
						coalesce ((SELECT Added FROM statistics WHERE TrackId = :track_id_add), :add_date),
						max (coalesce ((SELECT LastPlay FROM statistics where TrackId = :track_id_lp), 0), :play_date)
				);)");

		GetAlbumStats_ = QSqlQuery (DB_);
		GetAlbumStats_.prepare (R"(
				SELECT LastPlay, Name
				FROM statistics INNER JOIN tracks ON TrackId = Tracks.Id
				WHERE AlbumId = :album_id
				GROUP BY AlbumId
				HAVING LastPlay = MAX(LastPlay)
				)");

		GetArtistStats_ = QSqlQuery (DB_);
		GetArtistStats_.prepare (R"(
				SELECT LastPlay, albums.Name, tracks.Name
				FROM statistics
				INNER JOIN tracks ON TrackId = tracks.Id
				INNER JOIN albums ON albums.Id = tracks.AlbumId
				INNER JOIN artists2albums ON albums.Id = artists2albums.AlbumId
				INNER JOIN artists ON artists.Id = artists2albums.ArtistId
				WHERE artists.Id = :artist_id
				GROUP BY artists.Id
				HAVING LastPlay = MAX(LastPlay)
				)");

		GetFileIdMTime_ = QSqlQuery (DB_);
		GetFileIdMTime_.prepare ("SELECT MTime FROM fileTimes WHERE fileTimes.TrackID = :track_id;");

		GetFileMTime_ = QSqlQuery (DB_);
		GetFileMTime_.prepare ("SELECT MTime FROM fileTimes, tracks WHERE tracks.Path = :filepath AND tracks.Id = fileTimes.TrackID;");

		SetFileMTime_ = QSqlQuery (DB_);
		SetFileMTime_.prepare ("INSERT OR REPLACE INTO fileTimes (TrackID, MTime) VALUES ((SELECT Id FROM tracks WHERE Path = :filepath), :mtime);");

		GetLovedBanned_ = QSqlQuery (DB_);
		GetLovedBanned_.prepare ("SELECT Path FROM tracks INNER JOIN lovedBanned ON TrackId = Tracks.Id WHERE State = :state;");

		SetLovedBanned_ = QSqlQuery (DB_);
		SetLovedBanned_.prepare (R"(
				INSERT OR REPLACE INTO lovedBanned (TrackId, State)
				VALUES (:track_id, :state);
				)");

		RemoveLovedBanned_ = QSqlQuery (DB_);
		RemoveLovedBanned_.prepare ("DELETE FROM lovedBanned WHERE TrackId = :track_id;");

		GetOutdatedRgData_ = QSqlQuery (DB_);
		GetOutdatedRgData_.prepare (R"(
				SELECT DISTINCT tracks.AlbumID
				FROM tracks
				INNER JOIN fileTimes ON tracks.Id = fileTimes.TrackID
				LEFT OUTER JOIN rgdata ON fileTimes.TrackId = rgdata.TrackId
				WHERE fileTimes.MTime != rgdata.LastMTime OR rgdata.LastMTime IS NULL;
				)");

		GetTrackRgData_ = QSqlQuery (DB_);
		GetTrackRgData_.prepare (R"(
				SELECT TrackGain, TrackPeak, AlbumGain, AlbumPeak
				FROM rgdata, tracks
				WHERE tracks.Path = :filepath AND tracks.Id = rgdata.TrackId;
				)");

		SetTrackRgData_ = QSqlQuery (DB_);
		SetTrackRgData_.prepare (R"(
				INSERT OR REPLACE INTO rgdata
				(TrackId, LastMTime, TrackGain, TrackPeak, AlbumGain, AlbumPeak)
				VALUES
				(:track_id, :mtime, :track_gain, :track_peak, :album_gain, :album_peak);
				)");

		AppendToPlayHistory_ = QSqlQuery (DB_);
		AppendToPlayHistory_.prepare (R"(
				INSERT INTO playhistory (TrackId, Date)
				VALUES (:track_id, :date);
				)");
	}

	void LocalCollectionStorage::CreateTables ()
	{
		typedef QPair<QString, QString> QueryPair_t;
		QList<QueryPair_t> table2query;
		table2query << QueryPair_t ("artists",
				"CREATE TABLE artists ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"Name TEXT "
				");");
		table2query << QueryPair_t ("albums",
				"CREATE TABLE albums ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"Name TEXT, "
				"Year INTEGER, "
				"CoverPath TEXT "
				");");
		table2query << QueryPair_t ("artists2albums",
				"CREATE TABLE artists2albums ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"ArtistID INTEGER NOT NULL REFERENCES artists (Id) ON DELETE CASCADE, "
				"AlbumID INTEGER NOT NULL REFERENCES albums (Id) ON DELETE CASCADE "
				");");
		table2query << QueryPair_t ("tracks",
				"CREATE TABLE tracks ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"ArtistID INTEGER NOT NULL REFERENCES artists (Id) ON DELETE CASCADE, "
				"AlbumId NOT NULL REFERENCES albums (Id) ON DELETE CASCADE, "
				"Path TEXT NOT NULL, "
				"Name TEXT NOT NULL, "
				"TrackNumber INTEGER, "
				"Length INTEGER "
				");");
		table2query << QueryPair_t ("genres",
				"CREATE TABLE genres ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"TrackId NOT NULL REFERENCES tracks (Id) ON DELETE CASCADE, "
				"Name TEXT NOT NULL "
				");");
		table2query << QueryPair_t ("statistics",
				"CREATE TABLE statistics ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"TrackId NOT NULL UNIQUE REFERENCES tracks (Id) ON DELETE CASCADE, "
				"Playcount INTEGER, "
				"Added TIMESTAMP, "
				"LastPlay TIMESTAMP, "
				"Score INTEGER, "
				"Rating INTEGER "
				");");
		table2query << QueryPair_t ("lovedBanned",
				"CREATE TABLE lovedBanned ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"TrackId NOT NULL UNIQUE REFERENCES tracks (Id) ON DELETE CASCADE, "
				"State INTEGER"
				");");
		table2query << QueryPair_t ("fileTimes",
				"CREATE TABLE fileTimes ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"TrackID INTEGER UNIQUE NOT NULL REFERENCES tracks (Id) ON DELETE CASCADE, "
				"MTime TIMESTAMP NOT NULL"
				");");
		table2query << QueryPair_t ("rgdata",
				"CREATE TABLE rgdata ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"TrackId INTEGER UNIQUE NOT NULL REFERENCES tracks (Id) ON DELETE CASCADE, "
				"LastMTime TIMESTAMP NOT NULL, "
				"TrackGain DOUBLE NOT NULL, "
				"TrackPeak DOUBLE NOT NULL, "
				"AlbumGain DOUBLE NOT NULL, "
				"AlbumPeak DOUBLE NOT NULL "
				");");
		table2query << QueryPair_t ("playhistory",
				"CREATE TABLE playhistory ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"TrackId INTEGER NOT NULL REFERENCES tracks (Id) ON DELETE CASCADE, "
				"Date TIMESTAMP"
				");");
		table2query << QueryPair_t ("ignored_tracks",
				"CREATE TABLE ignored_tracks ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"TrackId INTEGER UNIQUE NOT NULL REFERENCES tracks (Id) ON DELETE CASCADE"
				");");

		Util::DBLock lock (DB_);
		lock.Init ();

		QSqlQuery { DB_ }.exec ("PRAGMA defer_foreign_keys = ON;");

		const auto& tables = DB_.tables ();
		for (const auto& pair : table2query)
			if (!tables.contains (pair.first))
			{
				QSqlQuery q (DB_);
				if (!q.exec (pair.second))
				{
					Util::DBLock::DumpError (q);
					throw std::runtime_error ("cannot create required tables");
				}
			}

		const auto tracksTableVersion = XmlSettingsManager::Instance ()
				.Property ("TracksTableVersion", 1).toInt ();
		if (tracksTableVersion < 2)
		{
			const QString query = "CREATE TABLE tracks2 ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"ArtistID INTEGER NOT NULL REFERENCES artists (Id) ON DELETE CASCADE, "
				"AlbumId NOT NULL REFERENCES albums (Id) ON DELETE CASCADE, "
				"Path TEXT NOT NULL UNIQUE, "
				"Name TEXT NOT NULL, "
				"TrackNumber INTEGER, "
				"Length INTEGER "
				");";

			QSqlQuery q { DB_ };
			if (!q.exec (query))
			{
				Util::DBLock::DumpError (q);
				throw std::runtime_error ("cannot create tracks2");
			}

			if (!q.exec ("INSERT OR IGNORE INTO tracks2 SELECT * FROM tracks;") ||
				!q.exec ("DROP TABLE tracks;") ||
				!q.exec ("ALTER TABLE tracks2 RENAME TO tracks;"))
			{
				Util::DBLock::DumpError (q);
				throw std::runtime_error ("cannot copy data from tracks2");
			}
			XmlSettingsManager::Instance ().setProperty ("TracksTableVersion", 2);
		}

		QSqlQuery (DB_).exec ("CREATE UNIQUE INDEX IF NOT EXISTS index_tracksPaths ON tracks (Path);");

		lock.Good ();
	}
}
