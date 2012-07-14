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
#include <interfaces/media/iartistbiofetcher.h>

namespace LeechCraft
{
namespace LMP
{
	class BioPropProxy : public QObject
	{
		Q_OBJECT

		Q_PROPERTY (QString artistName READ GetArtistName NOTIFY artistNameChanged);
		Q_PROPERTY (QUrl artistImageURL READ GetArtistImageURL NOTIFY artistImageURLChanged);
		Q_PROPERTY (QString artistTags READ GetArtistTags NOTIFY artistTagsChanged);
		Q_PROPERTY (QString artistInfo READ GetArtistInfo NOTIFY artistInfoChanged);

		Media::ArtistBio Bio_;

		QString CachedTags_;
		QString CachedInfo_;
	public:
		BioPropProxy (QObject* = 0);

		void SetBio (const Media::ArtistBio&);

		QString GetArtistName () const;
		QUrl GetArtistImageURL () const;
		QString GetArtistTags () const;
		QString GetArtistInfo () const;
	signals:
		void artistNameChanged (const QString&);
		void artistImageURLChanged (const QUrl&);
		void artistTagsChanged (const QString&);
		void artistInfoChanged (const QString&);
	};
}
}
