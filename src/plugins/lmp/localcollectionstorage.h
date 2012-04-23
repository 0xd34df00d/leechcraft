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
#include <QSqlDatabase>
#include <QSqlQuery>
#include "mediainfo.h"
#include "collectiontypes.h"

namespace LeechCraft
{
namespace LMP
{
	class LocalCollectionStorage : public QObject
	{
		Q_OBJECT

		QHash<QString, int> PresentArtists_;
		QHash<QString, int> PresentAlbums_;

		QSqlDatabase DB_;

		QSqlQuery GetArtists_;
		QSqlQuery GetAlbums_;
		QSqlQuery GetArtistAlbums_;
		QSqlQuery GetAlbumTracks_;
		QSqlQuery GetTrackGenres_;

		QSqlQuery AddArtist_;
		QSqlQuery AddAlbum_;
		QSqlQuery LinkArtistAlbum_;
		QSqlQuery AddTrack_;
		QSqlQuery AddGenre_;
	public:
		LocalCollectionStorage (QObject* = 0);

		void Clear ();
		Collection::Artists_t AddToCollection (const QList<MediaInfo>&);
		Collection::Artists_t Load ();
	private:
		Collection::Artists_t GetAllArtists ();
		QHash<int, Collection::Album_ptr> GetAllAlbums ();
		QList<Collection::Track> GetAlbumTracks (int);
		QStringList GetTrackGenres (int);

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
}
