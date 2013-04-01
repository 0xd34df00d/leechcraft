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

#include <QStringList>

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

		/** @brief Constructs an empty request.
		 */
		LyricsQuery ()
		{
		}

		/** @brief Constructs a search request for the given track data.
		 *
		 * @param[in] artist The artist name.
		 * @param[in] album The album (or single/EP/etc.) name.
		 * @param[in] title The title of the track.
		 */
		LyricsQuery (const QString& artist, const QString& album, const QString& title)
		: Artist_ (artist)
		, Album_ (album)
		, Title_ (title)
		{
		}
	};

	/** @brief Described the various lyrics request options.
	 *
	 * @sa ILyricsFinder
	 */
	enum QueryOption
	{
		/** @brief Default lyrics search request.
		 */
		NoOption = 0x0,

		/** @brief Refresh any cached data.
		 */
		Refresh = 0x1
	};

	/** @brief Typedef for <code>QFlags<QueryOption></code>.
	 */
	Q_DECLARE_FLAGS (QueryOptions, QueryOption);

	/** @brief Interface for plugins supporting finding lyrics.
	 *
	 * Plugins that support searching for lyrics should implement this
	 * interface.
	 *
	 * Fetching lyrics is asynchronous in nature, so one should request
	 * fetching the lyrics via RequestLyrics() method and wait for the
	 * gotLyrics() signal with the corresponding query parameter.
	 *
	 * @todo Consider migrating to handle-based results, like for
	 * IArtistBioFetcher.
	 */
	class Q_DECL_EXPORT ILyricsFinder
	{
	public:
		virtual ~ILyricsFinder () {}

		/** @brief Requests searching for lyrics for the given query.
		 *
		 * @param[in] query The lyrics query.
		 * @param[in] options Additional search options.
		 */
		virtual void RequestLyrics (const LyricsQuery& query, QueryOptions options = NoOption) = 0;
	protected:
		/** @brief Emitted when search for lyrics is complete.
		 *
		 * @param[out] query The query for which the search is complete.
		 * @param[out] lyrics The list of possible lyrics variants, may
		 * be empty or contain duplicates.
		 */
		virtual void gotLyrics (const LyricsQuery& query, const QStringList& lyrics) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::ILyricsFinder, "org.LeechCraft.Media.ILyricsFinder/1.0");
