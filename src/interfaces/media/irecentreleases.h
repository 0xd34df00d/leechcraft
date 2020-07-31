/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QDateTime>
#include <QUrl>
#include <util/sll/eitherfwd.h>

template<typename>
class QFuture;

namespace Media
{
	/** @brief Describes a recent release.
	 *
	 * This structure is used to describe a recent release that may be of
	 * interest to the user.
	 *
	 * @sa IRecentReleases
	 */
	struct AlbumRelease
	{
		/** @brief The release title.
		 */
		QString Title_;

		/** @brief The artist performing this release.
		 */
		QString Artist_;

		/** @brief The date of this release.
		 */
		QDateTime Date_;

		/** @brief Thumbnail image of this release (like album art).
		 */
		QUrl ThumbImage_;

		/** @brief Full-size image of this release.
		 */
		QUrl FullImage_;

		/** @brief The page describing this release in more details.
		 */
		QUrl ReleaseURL_;
	};

	/** @brief Interface for plugins providing recent releases.
	 *
	 * This interface should be implemented by plugins providing
	 * information about recent releases based on user's musical taste
	 * (like Last.FM's service).
	 */
	class Q_DECL_EXPORT IRecentReleases
	{
	public:
		virtual ~IRecentReleases () {}

		/** @brief The result of a recent releases query.
		 *
		 * The result of a recent releases query is either a string with a
		 * human-readable error text, or a list of AlbumRelease objects.
		 */
		using Result_t = LC::Util::Either<QString, QList<AlbumRelease>>;

		/** @brief Requests the recent releases.
		 *
		 * If withRecommends is set to false then only releases by the
		 * artists in the user's library should be fetched. Otherwise,
		 * the result set may include (or consist only of) releases that
		 * are recommended to the user (based on his musical taste, for
		 * example) but aren't directly related to artists in his library.
		 *
		 * @param[in] number The number of releases to get.
		 * @param[in] withRecommends Whether recommendations or releases
		 * from user's library should be fetched.
		 * @return The future holding the recent releases query result.
		 */
		virtual QFuture<Result_t> RequestRecentReleases (int number, bool withRecommends) = 0;

		/** @brief Returns the service name.
		 *
		 * This string returns a human-readable string with the service
		 * name, like "Last.FM".
		 *
		 * @return The human-readable service name.
		 */
		virtual QString GetServiceName () const = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IRecentReleases, "org.LeechCraft.Media.IRecentReleases/1.0")
