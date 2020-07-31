/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <boost/optional.hpp>
#include <QStringList>
#include <util/sll/eitherfwd.h>

template<typename>
class QFuture;

namespace Media
{
	/** @brief Describes a lyrics search request.
	 *
	 * @sa ILyricsFinder
	 */
	struct LyricsQuery
	{
		/** @brief The artist preforming the track.
		 */
		QString Artist_;

		/** @brief The album this track is on.
		 */
		QString Album_;

		/** @brief The title of the track.
		 */
		QString Title_;

		/** @brief The year of the album the track is on.
		 */
		boost::optional<int> Year_;

		/** @brief The track number.
		 */
		boost::optional<int> Track_;
	};

	/** @brief Describes a single lyrics result item.
	 *
	 * A result item is the lyrics themselves and the provider name these
	 * lyrics were fetched from.
	 *
	 * @sa LyricsResults
	 */
	struct LyricsResultItem
	{
		/** @brief The name of the provider lyrics were fetched from.
		 */
		QString ProviderName_;

		/** @brief The HTML-formatted lyrics string.
		 */
		QString Lyrics_;
	};

	/** @brief Describes the result set for a given lyrics query.
	 *
	 * @sa LyricsResultItem
	 */
	using LyricsResults = QList<LyricsResultItem>;

	/** @brief Interface for plugins supporting finding lyrics.
	 *
	 * Plugins that support searching for lyrics should implement this
	 * interface.
	 */
	class Q_DECL_EXPORT ILyricsFinder
	{
	public:
		virtual ~ILyricsFinder () {}

		/** @brief The result of a lyrics search query.
		 *
		 * The result of a lyrics search query is either a string with a
		 * human-readable error text, or a LyricsResults object.
		 *
		 * @sa LyricsResults
		 */
		using LyricsQueryResult_t = LC::Util::Either<QString, LyricsResults>;

		/** @brief Requests searching for lyrics for the given query.
		 *
		 * The returned future potentially provides multiple results.
		 *
		 * @param[in] query The lyrics query.
		 * @return The future (potentially providing multiple results) with the
		 * search results.
		 */
		virtual QFuture<LyricsQueryResult_t> RequestLyrics (const LyricsQuery& query) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::ILyricsFinder, "org.LeechCraft.Media.ILyricsFinder/1.0")
