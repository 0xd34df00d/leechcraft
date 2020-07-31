/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDateTime>
#include <util/sll/eitherfwd.h>
#include "audiostructs.h"

class QObject;

template<typename>
class QFuture;

namespace Media
{
	/** @brief Describes a single artist photo.
	 */
	struct ArtistImage
	{
		/** @brief The title of the image.
		 */
		QString Title_;

		/** @brief The author if the image.
		 */
		QString Author_;

		/** @brief The date and time the image was taken.
		 */
		QDateTime Date_;

		/** @brief URL of the thumbnail version of the image.
		 */
		QUrl Thumb_;

		/** @brief URL of the full version of the image.
		 */
		QUrl Full_;
	};

	/** @brief Information about artist biography.
	 *
	 * For now this structure only contains basic information about the
	 * artist, duplicating ArtistInfo. This may be changed/extended some
	 * time in the future, though.
	 *
	 * @sa ArtistInfo
	 */
	struct ArtistBio
	{
		/** @brief Basic information about this artist.
		 */
		ArtistInfo BasicInfo_;

		/** @brief Other images for this artist.
		 *
		 * This list will typically include posters, concerts photos and
		 * similar stuff.
		 */
		QList<ArtistImage> OtherImages_;
	};

	/** @brief Interface for plugins supporting fetching artist biography.
	 *
	 * Plugins that support fetching artist biography from the sources
	 * Last.FM should implement this interface.
	 */
	class Q_DECL_EXPORT IArtistBioFetcher
	{
	public:
		virtual ~IArtistBioFetcher () {}

		/** @brief The result of an artist biography search query.
		 *
		 * The result of an artist biography search query is either a string with a
		 * human-readable error text, or a ArtistBio object.
		 *
		 * @sa ArtistBio
		 */
		using Result_t = LC::Util::Either<QString, ArtistBio>;

		/** @brief Returns the service name.
		 *
		 * This string returns a human-readable string with the service
		 * name, like "Last.FM".
		 *
		 * @return The human-readable service name.
		 */
		virtual QString GetServiceName () const = 0;

		/** @brief Requests the biography of the given artist.
		 *
		 * This function initiates a search for artist biography and
		 * returns a future with the biography search result.
		 *
		 * @param[in] artist The artist name.
		 * @param[in] additionalImages Whether additional images for the
		 * ArtistBio::OtherImages_ field should be requested.
		 * @return The pending biography future.
		 */
		virtual QFuture<Result_t> RequestArtistBio (const QString& artist, bool additionalImages = true) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IArtistBioFetcher, "org.LeechCraft.Media.IArtistBioFetcher/1.0")
