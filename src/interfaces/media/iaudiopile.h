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
#include "audiostructs.h"

class QObject;

namespace Media
{
	/** @brief Pending audio search handle.
	 *
	 * Interface for a handle to a pending audio search in an IAudioPile.
	 * A descendant of this class is returned from IAudioPile::Search()
	 * and is used to track the status of audio search requests.
	 *
	 * This class has some signals (ready() and error()), and one can use
	 * the GetQObject() method to get an object of this class as a
	 * QObject and connect to those signals.
	 *
	 * @note The object of this class should schedule its deletion (via
	 * <code>QObject::deleteLater()</code>, for example) after ready() or
	 * error() signal is emitted. Thus the calling code should never
	 * delete it explicitly, neither it should use this object after
	 * ready() or error() signals or connect to this signals via
	 * <code>Qt::QueuedConnection</code>.
	 *
	 * @sa IAudioPile
	 */
	class Q_DECL_EXPORT IPendingAudioSearch
	{
	public:
		virtual ~IPendingAudioSearch () {}

		virtual QObject* GetQObject () = 0;

		/** @brief A structure describing a single entry in search result.
		 */
		struct Result
		{
			/** The information about the found audio track.
			 */
			AudioInfo Info_;

			/** The URL of this audio track.
			 */
			QUrl Source_;
		};

		/** @brief Returns the list of audio tracks.
		 *
		 * This function returns the list of audio tracks that were found
		 * during this search, or an empty list if no tracks are found,
		 * an error occured on the search isn't finished yet.
		 *
		 * @return The list of found audio tracks.
		 */
		virtual QList<Result> GetResults () const = 0;
	protected:
		/** @brief Emitted when the search is completed without error.
		 *
		 * The object will be invalid after this signal is emitted and
		 * the event loop is run.
		 */
		virtual void ready () = 0;

		/** @brief Emitted when the search is cancelled due to errors.
		 *
		 * Empty result set is \em not an error. ready() will be
		 * emitted in that case.
		 *
		 * The object will be invalid after this signal is emitted and
		 * the event loop is run.
		 */
		virtual void error () = 0;
	};

	/** @brief Describes a request for an audio search in an IAudioPile.
	 *
	 * Different audio piles support filtering by different criteria, so
	 * calling plugins should not rely on all criteria being fulfilled.
	 * But at least filtering by either title, artist or free form
	 * request should be supported.
	 *
	 * @sa IAudioPile
	 */
	struct AudioSearchRequest
	{
		/** The title of a track.
		 *
		 * At least this or Artist_ field should not be empty.
		 */
		QString Title_;

		/** The artist performing the track.
		 *
		 * At least this or Title_ field should not be empty.
		 */
		QString Artist_;

		/** The album containing this track.
		 */
		QString Album_;

		/** @brief The approximate length of the track.
		 *
		 * Set this to 0 to disable by-track filtering.
		 */
		int TrackLength_;

		/** @brief Free form engine-specific request.
		 *
		 * Calling plugins should set this instead of Title_ or Artist_
		 * fields if they are not sure what user has entered.
		 */
		QString FreeForm_;

		AudioSearchRequest ()
		: TrackLength_ (0)
		{
		}
	};

	/** @brief Interface for plugins supporting searching for tracks.
	 *
	 * Plugins that support searching for audio tracks in huge loosely
	 * categorized audio collections like VKontakte should implement this
	 * interface.
	 */
	class Q_DECL_EXPORT IAudioPile
	{
	public:
		virtual ~IAudioPile () {}

		/** @brief Requests a search by the given request.
		 *
		 * This function initiates a search by the given request and
		 * returns a handle that can be used to track the search result
		 * state. The handle owns itself and deletes itself after the
		 * results are available — see its documentation for details.
		 *
		 * @param[in] request The structure describing the search request.
		 * @return The pending audio search handle.
		 */
		virtual IPendingAudioSearch* Search (const AudioSearchRequest& request) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IPendingAudioSearch, "org.LeechCraft.Media.IPendingAudioSearch/1.0");
Q_DECLARE_INTERFACE (Media::IAudioPile, "org.LeechCraft.Media.IAudioPile/1.0");
