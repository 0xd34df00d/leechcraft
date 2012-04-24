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

#include "localcollectionstorage.h"
#include <stdexcept>
#include <QSqlError>
#include <QSqlQuery>
#include <util/util.h>
#include <util/dblock.h>
#include "util.h"

namespace LeechCraft
{
namespace LMP
{
	LocalCollectionStorage::LocalCollectionStorage (QObject *parent)
	: QObject (parent)
	, DB_ (QSqlDatabase::addDatabase ("QSQLITE", QString ("LMP_LocalCollection_%1").arg (qrand ())))
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
		Q_FOREACH (const MediaInfo& info, infos)
		{
			Collection::Artist artist =
			{
				0,
				info.Artist_,
				QList<Collection::Album_ptr> ()
			};
			if (!IsPresent (artist, artist.ID_))
			{
				AddArtist (artist);
				artists [artist.ID_] = artist;
			}

			Collection::Album album =
			{
				0,
				info.Album_,
				info.Year_,
				QString (),
				QList<Collection::Track> ()
			};
			if (!IsPresent (artist, album, album.ID_))
			{
				album.CoverPath_ = FindAlbumArtPath (info.LocalPath_);
				AddAlbum (artist, album);
				artists [artist.ID_].Albums_ << Collection::Album_ptr (new Collection::Album (album));
			}

			Collection::Track track =
			{
				0,
				info.TrackNumber_,
				info.Title_,
				info.Length_,
				info.Genres_,
				info.LocalPath_
			};
			AddTrack (track, artist.ID_, album.ID_);

			auto& trackArtist = artists [artist.ID_];
			for (auto i = trackArtist.Albums_.begin (), end = trackArtist.Albums_.end (); i != end; ++i)
				if ((*i)->ID_ == album.ID_)
				{
					(*i)->Tracks_ << track;
					break;
				}
		}
		lock.Good ();

		return artists.values ();
	}

	Collection::Artists_t LocalCollectionStorage::Load ()
	{
		Collection::Artists_t artists = GetAllArtists ();
		const auto& albums = GetAllAlbums ();

		for (auto i = artists.begin (), end = artists.end (); i != end; ++i)
		{
			AddToPresent (*i);

			GetArtistAlbums_.bindValue (":artist_id", i->ID_);
			if (!GetArtistAlbums_.exec ())
			{
				Util::DBLock::DumpError (GetArtists_);
				throw std::runtime_error ("cannot fetch artists");
			}
			while (GetArtistAlbums_.next ())
			{
				const int albumID = GetArtistAlbums_.value (0).toInt ();
				auto album = albums [albumID];
				i->Albums_ << album;
				AddToPresent (*i, *album);
			}
		}
		GetArtistAlbums_.finish ();

		return artists;
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
		QHash<int, Collection::Album_ptr> albums;

		if (!GetAlbums_.exec ())
		{
			Util::DBLock::DumpError (GetAlbums_);
			throw std::runtime_error ("cannot fetch albums");
		}
		while (GetAlbums_.next ())
		{
			const int albumID = GetAlbums_.value (0).toInt ();
			Collection::Album a =
			{
				albumID,
				GetAlbums_.value (1).toString (),
				GetAlbums_.value (2).toInt (),
				GetAlbums_.value (3).toString (),
				GetAlbumTracks (albumID)
			};

			albums [albumID] = Collection::Album_ptr (new Collection::Album (a));
		}
		GetAlbums_.finish ();

		return albums;
	}

	QList<Collection::Track> LocalCollectionStorage::GetAlbumTracks (int albumId)
	{
		QList<Collection::Track> tracks;

		GetAlbumTracks_.bindValue (":album_id", albumId);
		if (!GetAlbumTracks_.exec ())
		{
			Util::DBLock::DumpError (GetAlbumTracks_);
			throw std::runtime_error ("cannot fetch album tracks");
		}
		while (GetAlbumTracks_.next ())
		{
			const int trackId = GetAlbumTracks_.value (0).toInt ();
			Collection::Track t =
			{
				trackId,
				GetAlbumTracks_.value (1).toInt (),
				GetAlbumTracks_.value (2).toString (),
				GetAlbumTracks_.value (3).toInt (),
				GetTrackGenres (trackId),
				GetAlbumTracks_.value (4).toString ()
			};
			tracks << t;
		}
		GetAlbumTracks_.finish ();

		return tracks;
	}

	QStringList LocalCollectionStorage::GetTrackGenres (int trackId)
	{
		QStringList genres;

		GetTrackGenres_.bindValue (":track_id", trackId);
		if (!GetTrackGenres_.exec ())
		{
			Util::DBLock::DumpError (GetTrackGenres_);
			throw std::runtime_error ("cannot fetch track genres");
		}
		while (GetTrackGenres_.next ())
			genres << GetTrackGenres_.value (0).toString ();
		GetTrackGenres_.finish ();

		return genres;
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

		Q_FOREACH (const QString& genre, track.Genres_)
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

		GetArtistAlbums_ = QSqlQuery (DB_);
		GetArtistAlbums_.prepare ("SELECT albums.Id FROM albums INNER JOIN artists2albums ON albums.Id = artists2albums.AlbumId WHERE artists2albums.ArtistId = :artist_id;");

		GetAlbumTracks_ = QSqlQuery (DB_);
		GetAlbumTracks_.prepare ("SELECT Id, TrackNumber, Name, Length, Path FROM tracks WHERE AlbumId = :album_id;");

		GetTrackGenres_ = QSqlQuery (DB_);
		GetTrackGenres_.prepare ("SELECT Name FROM genres WHERE TrackId = :track_id;");

		AddArtist_ = QSqlQuery (DB_);
		AddArtist_.prepare ("INSERT INTO artists (Name) VALUES (:name);");

		AddAlbum_ = QSqlQuery (DB_);
		AddAlbum_.prepare ("INSERT INTO albums (Name, Year, CoverPath) VALUES (:name, :year, :cover_path);");

		LinkArtistAlbum_ = QSqlQuery (DB_);
		LinkArtistAlbum_.prepare ("INSERT INTO artists2albums (ArtistID, AlbumID) VALUES (:artist_id, :album_id);");

		AddTrack_ = QSqlQuery (DB_);
		AddTrack_.prepare ("INSERT INTO tracks (ArtistID, AlbumID, Path, Name, TrackNumber, Length) "
				"VALUES (:artist_id, :album_id, :path, :name, :track_number, :length);");

		AddGenre_ = QSqlQuery (DB_);
		AddGenre_.prepare ("INSERT INTO genres (TrackId, Name) VALUES (:track_id, :name);");
	}

	void LocalCollectionStorage::CreateTables ()
	{
		QMap<QString, QString> table2query;
		table2query ["artists"] = "CREATE TABLE artists ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"Name TEXT "
				");";
		table2query ["albums"] = "CREATE TABLE albums ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"Name TEXT, "
				"Year INTEGER, "
				"CoverPath TEXT "
				");";
		table2query ["artists2albums"] = "CREATE TABLE artists2albums ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"ArtistID INTEGER NOT NULL REFERENCES artists (Id) ON DELETE CASCADE, "
				"AlbumID INTEGER NOT NULL REFERENCES albums (Id) ON DELETE CASCADE "
				");";
		table2query ["tracks"] = "CREATE TABLE tracks ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"ArtistID INTEGER NOT NULL REFERENCES artists (Id) ON DELETE CASCADE, "
				"AlbumId NOT NULL REFERENCES albums (Id) ON DELETE CASCADE, "
				"Path TEXT NOT NULL, "
				"Name TEXT NOT NULL, "
				"TrackNumber INTEGER, "
				"Length INTEGER "
				");";
		table2query ["genres"] = "CREATE TABLE genres ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"TrackId NOT NULL REFERENCES tracks (Id) ON DELETE CASCADE, "
				"Name TEXT NOT NULL "
				");";
		table2query ["statistics"] = "CREATE TABLE statistics ("
				"Id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"TrackId NOT NULL REFERENCES tracks (Id) ON DELETE CASCADE, "
				"Playcount INTEGER, "
				"Added TIMESTAMP, "
				"LastPlay TIMESTAMP, "
				"Score INTEGER, "
				"Rating INTEGER "
				");";

		Util::DBLock lock (DB_);

		lock.Init ();

		const auto& tables = DB_.tables ();
		Q_FOREACH (const QString& key, table2query.keys ())
			if (!tables.contains (key))
			{
				QSqlQuery q (DB_);
				if (!q.exec (table2query [key]))
				{
					Util::DBLock::DumpError (q);
					throw std::runtime_error ("cannot create required tables");
				}
			}

		lock.Good ();
	}
}
}
