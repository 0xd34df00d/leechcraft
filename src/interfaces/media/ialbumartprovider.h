/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QList>
#include <QImage>
#include <QHash>
#include <QMetaType>
#include <util/sll/eitherfwd.h>

class QUrl;

template<typename>
class QFuture;

namespace Media
{
	/** @brief Information about an album used in IAlbumArtProvider.
	 */
	struct AlbumInfo
	{
		/** @brief The artist name of this album.
		 */
		QString Artist_;

		/** @brief The album name.
		 */
		QString Album_;

		/** @brief Compares two AlbumInfo structures.
		 */
		bool operator== (const AlbumInfo&) const = default;
	};

	/** @brief A hash function for AlbumInfo to use it with QHash.
	 */
	inline size_t qHash (const AlbumInfo& info)
	{
		const std::initializer_list<QString> elems { info.Album_, info.Artist_ };
		return qHashRange (elems.begin (), elems.end ());
	}

	/** @brief Interface for plugins that can search for album art.
	 *
	 * Plugins that can search for album art (like on Amazon or Last.FM)
	 * should implement this interface.
	 */
	class Q_DECL_EXPORT IAlbumArtProvider
	{
	protected:
		virtual ~IAlbumArtProvider () = default;
	public:
		/** @brief The result of an album art search query.
		 *
		 * The result of an album art search query is either a string with a
		 * human-readable error text, or a list of URLs matching the album art.
		 */
		using Result_t = LC::Util::Either<QString, QList<QUrl>>;

		/** @brief Returns the human-readable name of this provider.
		 *
		 * @return The human-readable name of the provider, like Last.FM.
		 */
		virtual QString GetAlbumArtProviderName () const = 0;

		/** @brief Initiates search for album art of the given album.
		 *
		 * This function initiates searching for the album art of the
		 * given \em album and returns a future with the album art search
		 * result.
		 *
		 * @param[in] album The description of the album.
		 * @return The future with the album art search result.
		 */
		virtual QFuture<Result_t> RequestAlbumArt (const AlbumInfo& album) const = 0;
	};
}

Q_DECLARE_METATYPE (Media::AlbumInfo)
Q_DECLARE_INTERFACE (Media::IAlbumArtProvider, "org.LeechCraft.Media.IAlbumArtProvider/1.0")
