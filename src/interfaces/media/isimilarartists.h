/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QtPlugin>
#include "audiostructs.h"

template<typename>
class QFuture;

namespace Media
{
	/** @brief Interface for plugins supporting similar artists.
	 *
	 * If a plugin supports fetching artists similar to another one it
	 * should implement this interface.
	 *
	 * @sa IRecommendedArtists
	 */
	class Q_DECL_EXPORT ISimilarArtists
	{
	public:
		virtual ~ISimilarArtists () {}

		/** @brief Requests the recommended artists.
		 *
		 * This function initiates request for the list of artists
		 * similar to a given one and returns a handle through which the
		 * results of this search could be obtained. The handle owns
		 * itself and deletes itself after results are available — see
		 * its documentation for more details.
		 *
		 * The results of the returned handle will typically have only
		 * SimilarityInfo::Similarity_ field set, while
		 * SimilarityInfo::SimilarTo_ field is unset. The Similarity
		 * field should be interpreted as some kind of "match percentage"
		 * displaying how much two artists resemble each other.
		 *
		 * @param[in] artistName The name of the artist for which to
		 * fetch similar artists.
		 * @param[in] count The number of recommended artists to fetch.
		 */
		virtual QFuture<SimilarityQueryResult_t> GetSimilarArtists (const QString& artistName, int count) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::ISimilarArtists, "org.LeechCraft.Media.ISimilarArtists/1.0")
