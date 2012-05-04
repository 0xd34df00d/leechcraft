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

#include <QString>
#include <QList>
#include <QImage>
#include <QHash>
#include <QMetaType>

namespace Media
{
	struct AlbumInfo
	{
		QString Artist_;
		QString Album_;
	};

	inline bool operator== (const AlbumInfo& a1, const AlbumInfo& a2)
	{
		return a1.Artist_ == a2.Artist_ &&
			a1.Album_ == a2.Album_;
	}

	inline uint qHash (const AlbumInfo& info)
	{
		return qHash (info.Album_.toUtf8 () + '\0' + info.Artist_.toUtf8 ());
	}

	class IAlbumArtProvider
	{
	public:
		virtual ~IAlbumArtProvider () {}

		virtual QString GetAlbumArtProviderName () const = 0;
		virtual void RequestAlbumArt (const AlbumInfo& album) const = 0;
	protected:
		virtual void gotAlbumArt (const AlbumInfo& album, const QList<QImage>& arts) = 0;
	};
}

Q_DECLARE_METATYPE (Media::AlbumInfo);
Q_DECLARE_INTERFACE (Media::IAlbumArtProvider, "org.LeechCraft.Media.IAlbumArtProvider/1.0");
