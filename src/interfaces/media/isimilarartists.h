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
#include <QtPlugin>

namespace Media
{
	class IPendingSimilarArtists;

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
		virtual IPendingSimilarArtists* GetSimilarArtists (const QString& artistName, int count) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::ISimilarArtists, "org.LeechCraft.Media.ISimilarArtists/1.0");
