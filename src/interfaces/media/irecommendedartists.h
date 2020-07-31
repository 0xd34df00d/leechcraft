/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>
#include "audiostructs.h"

template<typename>
class QFuture;

namespace Media
{
	/** @brief Interface for plugins supporting recommended artists.
	 *
	 * If a plugin supports fetching information about recommended
	 * artists, for example, based on user's musical taste, it should
	 * implement this interface.
	 *
	 * @sa ISimilarArtists
	 */
	class Q_DECL_EXPORT IRecommendedArtists
	{
	public:
		virtual ~IRecommendedArtists () {}

		/** @brief Requests the recommended artists.
		 *
		 * This function initiates request for the list of recommended
		 * artists for our user and returns a handle through which the
		 * results of this search could be obtained. The handle owns
		 * itself and deletes itself after results are available — see
		 * its documentation for more details.
		 *
		 * The results of the returned handle will typically have only
		 * SimilarityInfo::SimilarTo_ field set, while
		 * SimilarityInfo::Similarity_ field may be unset. Though if the
		 * latter is set it should be interpreted as some kind of "match
		 * percentage" displaying how interesting an artist can be to our
		 * user.
		 *
		 * Also, IPendingSimilarArtists::GetSourceArtistName() can return
		 * an empty string in this case since it there is no source
		 * artist for which recommendations are fetched.
		 *
		 * @param[in] count The number of recommended artists to fetch.
		 */
		virtual QFuture<SimilarityQueryResult_t> RequestRecommended (int count) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IRecommendedArtists, "org.LeechCraft.Media.IRecommendedArtists/1.0")
