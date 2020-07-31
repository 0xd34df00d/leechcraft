/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/sll/either.h>
#include "audiostructs.h"

template<typename>
class QFuture;

namespace Media
{
	/** @brief Information about a track release.
	 *
	 * A release can be, for example, an album, a compilation or a
	 * single.
	 *
	 * @sa IDiscographyProvider
	 */
	struct ReleaseTrackInfo
	{
		/** @brief The number of the track in the release.
		 */
		int Number_;

		/** @brief The name of the track.
		 */
		QString Name_;

		/** @brief The length of the track in this release.
		 */
		int Length_;
	};

	/** @brief Information about a release, like an album or a single.
	 *
	 * @sa IDiscographyProvider
	 */
	struct ReleaseInfo
	{
		/** @brief The internal ID of this release.
		 */
		QString ID_;

		/** @brief The name of this release.
		 */
		QString Name_;

		/** @brief The year of this release.
		 */
		int Year_;

		/** @brief The enum describing the recognized types of the releases.
		 */
		enum class Type
		{
			/** @brief A typical album.
			 */
			Standard,

			/** @brief An EP.
			 */
			EP,

			/** @brief A single track release.
			 */
			Single,

			/** @brief A compilation.
			 */
			Compilation,

			/** @brief A live release.
			 */
			Live,

			/** @brief A soundtrack.
			 */
			Soundtrack,

			/** @brief Some other release type currently unrecognized by
			 * LeechCraft.
			 */
			Other
		} Type_;

		/** List of tracks in this release.
		 */
		QList<QList<ReleaseTrackInfo>> TrackInfos_;
	};

	/** @brief Interface for plugins supporting getting artist discography.
	 *
	 * Plugins that support fetching artists discography from sources
	 * like MusicBrainz should implement this interface.
	 *
	 * Discography includes various types of releases (albums, EPs,
	 * singles, etc) as well as the corresponding lists of tracks in
	 * those releases.
	 */
	class Q_DECL_EXPORT IDiscographyProvider
	{
	public:
		virtual ~IDiscographyProvider () {}

		/** @brief The result of an audio search query.
		 *
		 * The result of an audio search query is either a string with a
		 * human-readable error text, or a list of result items.
		 *
		 * @sa Results_t
		 */
		using Result_t = LC::Util::Either<QString, QList<ReleaseInfo>>;

		/** @brief Returns the service name.
		 *
		 * This string returns a human-readable string with the service
		 * name, like "MusicBrainz".
		 *
		 * @return The human-readable service name.
		 */
		virtual QString GetServiceName () const = 0;

		/** @brief Fetches all the discography of the given artist.
		 *
		 * This function initiates a search for artist discography and
		 * returns a handle through which the results of the search could
		 * be obtained.
		 *
		 * All known releases of this artist are returned through the
		 * handle.
		 *
		 * The handle owns itself and deletes itself after results are
		 * available — see its documentation for details.
		 *
		 * @param[in] artist The artist name.
		 * @return The pending discography search handle.
		 */
		virtual QFuture<Result_t> GetDiscography (const QString& artist, const QStringList& hints) = 0;

		/** @brief Fetches contents of the given release by the artist.
		 *
		 * This function initiates a search for the given release of the
		 * given artist and returns a release query result future.
		 *
		 * @param[in] artist The artist name.
		 * @param[in] release The release name to search for.
		 * @return The pending discography search future.
		 */
		virtual QFuture<Result_t> GetReleaseInfo (const QString& artist, const QString& release) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IDiscographyProvider, "org.LeechCraft.Media.IDiscographyProvider/1.0")
