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
#include "audiostructs.h"

namespace Media
{
	/** @brief Pending tags fetch handle.
	 *
	 * Interface to a pending tags fetch job. A descendant of this class
	 * is returned from ITagsFetcher::FetchTags() method.
	 *
	 * This class has some signals (ready()), and one can use
	 * the GetQObject() method to get an object of this class as a
	 * QObject and connect to those signals.
	 *
	 * @note The object of this class should schedule its deletion (via
	 * <code>QObject::deleteLater()</code>, for example) after ready()
	 * signal is emitted. Thus the calling code should never
	 * delete it explicitly, neither it should use this object after
	 * ready() signal or connect to this signals via
	 * <code>Qt::QueuedConnection</code>.
	 *
	 * @sa ITagsFetcher
	 */
	class Q_DECL_EXPORT IPendingTagsFetch
	{
	public:
		virtual ~IPendingTagsFetch () {}

		/** @brief Returns this object as a QObject.
		 *
		 * This function can be used to connect to the signals of this
		 * class.
		 *
		 * @return This object as a QObject.
		 */
		virtual QObject* GetQObject () = 0;

		/** @brief Returns the found audio metadata.
		 *
		 * Returns the best matching audio info, if any, for the file
		 * this tags fetch job corresponds to.
		 *
		 * @return The result of the tags fetching.
		 */
		virtual AudioInfo GetResult () const = 0;
	protected:
		/** @brief Emitted when the search result if ready and fetched.
		 *
		 * The object will be invalid after this signal is emitted and
		 * the event loop is run.
		 *
		 * @param[out] filename The filename this tags fetch job
		 * corresponds to.
		 * @param[out] info The audio track metadata fetched for the
		 * filename.
		 */
		virtual void ready (const QString& filename, const Media::AudioInfo& info) = 0;
	};

	/** @brief Interface for plugins fetching tags for untagged files.
	 *
	 * If a plugin is able to fetch tags from a tags database by, for
	 * example, an audio fingerprint (like AcoustID/MusicBrainz database)
	 * it should implement this interface.
	 */
	class Q_DECL_EXPORT ITagsFetcher
	{
	public:
		virtual ~ITagsFetcher () {}

		/** @brief Requests fetching tags for the given file.
		 *
		 * This function initiates a search for tags for the given file
		 * and returns a handle that can be used to obtain the results.
		 * The handle owns itself and deletes itself after results are
		 * available — see its documentation for details.
		 *
		 * @param[in] filename The name of the file to search tags for.
		 * @return The pending tags fetch handle.
		 */
		virtual IPendingTagsFetch* FetchTags (const QString& filename) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IPendingTagsFetch, "org.LeechCraft.Media.IPendingTagsFetch/1.0");
Q_DECLARE_INTERFACE (Media::ITagsFetcher, "org.LeechCraft.Media.ITagsFetcher/1.0");
