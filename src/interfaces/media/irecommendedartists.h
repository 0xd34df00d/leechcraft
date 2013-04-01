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

#include <QtPlugin>

namespace Media
{
	class IPendingSimilarArtists;

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
		virtual IPendingSimilarArtists* RequestRecommended (int count) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IRecommendedArtists, "org.LeechCraft.Media.IRecommendedArtists/1.0");
