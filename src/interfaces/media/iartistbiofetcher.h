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

#include "audiostructs.h"

class QObject;

namespace Media
{
	/** @brief Information about artist biography.
	 *
	 * For now this structure only contains basic information about the
	 * artist, duplicating ArtistInfo. This may be changed/extended some
	 * time in the future, though.
	 *
	 * @sa ArtistInfo
	 */
	struct ArtistBio
	{
		/** Basic information about this artist.
		 */
		ArtistInfo BasicInfo_;
	};

	/** @brief Pending biography request handle.
	 *
	 * Interface to a pending biography search in an IArtistBioFetcher. A
	 * descendant of this class is returned from the
	 * IArtistBioFetcher::RequestArtistBio() method and is used to track
	 * the status of biography requests.
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
	 * @sa IArtistBioFetcher
	 */
	class Q_DECL_EXPORT IPendingArtistBio
	{
	public:
		virtual ~IPendingArtistBio () {}

		/** @brief Returns this object as a QObject.
		 *
		 * This function can be used to connect to the signals of this
		 * class.
		 *
		 * @return This object as a QObject.
		 */
		virtual QObject* GetQObject () = 0;

		/** @brief Returns the artist biography.
		 *
		 * This function returns the fetched artist biography, or an
		 * empty biography if it is not found or search isn't completed
		 * yet.
		 *
		 * @return The fetched artist biography.
		 */
		virtual ArtistBio GetArtistBio () const = 0;
	protected:
		/** @brief Emitted when the biography is ready and fetched.
		 *
		 * The object will be invalid after this signal is emitted and
		 * the event loop is run.
		 */
		virtual void ready () = 0;

		/** @brief Emitted when there is an error fetching the biography.
		 *
		 * The object will be invalid after this signal is emitted and
		 * the event loop is run.
		 */
		virtual void error () = 0;
	};

	/** @brief Interface for plugins supporting fetching artist biography.
	 *
	 * Plugins that support fetching artist biography from the sources
	 * Last.FM should implement this interface.
	 */
	class Q_DECL_EXPORT IArtistBioFetcher
	{
	public:
		virtual ~IArtistBioFetcher () {}

		/** @brief Returns the service name.
		 *
		 * This string returns a human-readable string with the service
		 * name, like "Last.FM".
		 *
		 * @return The human-readable service name.
		 */
		virtual QString GetServiceName () const = 0;

		/** @brief Requests the biography of the given artist.
		 *
		 * This function initiates a search for artist biography and
		 * returns a handle through which the results of the search could
		 * be obtained. The handle owns itself and deletes itself after
		 * results are available — see its documentation for details.
		 *
		 * @param[in] artist The artist name.
		 * @return The pending biography search handle.
		 */
		virtual IPendingArtistBio* RequestArtistBio (const QString& artist) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IPendingArtistBio, "org.LeechCraft.Media.IPendingArtistBio/1.0");
Q_DECLARE_INTERFACE (Media::IArtistBioFetcher, "org.LeechCraft.Media.IArtistBioFetcher/1.0");
