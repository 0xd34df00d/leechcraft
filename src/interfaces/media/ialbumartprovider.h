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
#include <QMetaType>
#include <QtPlugin>
#include <util/threads/coro/channel.h>
#include <util/sll/either.h>

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
		return qHash (std::pair { info.Album_, info.Artist_ });
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
		struct AlbumArtResponse
		{
			/** @brief The human-readable name of the service, like `"Last.FM"`.
			 */
			QString ServiceName_;

			using Result_t = LC::Util::Either<QString, QList<QUrl>>;

			/** @brief The result of an album art search query.
			 *
			 * The result of an album art search query is either a string with a
			 * human-readable error text, or a list of URLs matching the album art.
			 */
			Result_t Result_;
		};

		using Channel_t = LC::Util::Channel_ptr<AlbumArtResponse>;

		/** @brief Initiates search for album art of the given album.
		 *
		 * This function initiates searching for the album art of the
		 * given \em album and returns a channel for the album art search
		 * results.
		 *
		 * @param[in] album The description of the album.
		 * @return The channel with the album art search results.
		 */
		[[nodiscard]]
		virtual Channel_t RequestAlbumArt (const AlbumInfo& album) const = 0;
	};
}

Q_DECLARE_METATYPE (Media::AlbumInfo)
Q_DECLARE_INTERFACE (Media::IAlbumArtProvider, "org.LeechCraft.Media.IAlbumArtProvider/1.0")
