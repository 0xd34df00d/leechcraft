/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
	/** @brief Information about an album used in IAlbumArtProvider.
	 */
	struct AlbumInfo
	{
		/** The artist name of this album.
		 */
		QString Artist_;

		/** The album name.
		 */
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

	/** @brief Interface for plugins that can search for album art.
	 *
	 * Plugins that can search for album art (like on Amazon or Last.FM)
	 * should implement this interface.
	 *
	 * Album art lookup is asynchronous in nature: one first initiates a
	 * search via RequestAlbumArt() method and then listens for the
	 * gotAlbumArt() signal.
	 */
	class Q_DECL_EXPORT IAlbumArtProvider
	{
	public:
		virtual ~IAlbumArtProvider () {}

		/** @brief Returns the human-readable name of this provider.
		 *
		 * @return The human-readable name of the provider, like Last.FM.
		 */
		virtual QString GetAlbumArtProviderName () const = 0;

		/** @brief Initiates search for album art of the given album.
		 *
		 * @param[in] album The information about the album.
		 */
		virtual void RequestAlbumArt (const AlbumInfo& album) const = 0;
	protected:
		/** @brief Emitted when album art for the given album is
		 * available.
		 *
		 * This signal should be emitted by the implementation even if
		 * no album art is found for the given album.
		 *
		 * @param[in] album The album for which the album art is found.
		 * @param[in] arts The list of album covers that were found. The
		 * list may be empty.
		 */
		virtual void gotAlbumArt (const AlbumInfo& album, const QList<QImage>& arts) = 0;
	};
}

Q_DECLARE_METATYPE (Media::AlbumInfo);
Q_DECLARE_INTERFACE (Media::IAlbumArtProvider, "org.LeechCraft.Media.IAlbumArtProvider/1.0");
