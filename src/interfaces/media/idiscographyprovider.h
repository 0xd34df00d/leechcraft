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

namespace Media
{
	/** @brief Information about a track release.
	 *
	 * A release can be, for example, an album, a compilation or a
	 * single.
	 *
	 * @sa IDiscographyProvider
	 */
	struct ReleaseTrackInfo
	{
		/** @brief The number of the track in the release.
		 */
		int Number_;

		/** @brief The name of the track.
		 */
		QString Name_;

		/** @brief The length of the track in this release.
		 */
		int Length_;
	};

	/** @brief Information about a release, like an album or a single.
	 *
	 * @sa IDiscographyProvider
	 */
	struct ReleaseInfo
	{
		/** @brief The internal ID of this release.
		 */
		QString ID_;

		/** @brief The name of this release.
		 */
		QString Name_;

		/** @brief The year of this release.
		 */
		int Year_;

		/** @brief The enum describing the recognized types of the releases.
		 */
		enum class Type
		{
			/** @brief A typical album.
			 */
			Standard,

			/** @brief An EP.
			 */
			EP,

			/** @brief A single track release.
			 */
			Single,

			/** @brief Some other release type currently unrecognized by
			 * LeechCraft.
			 */
			Other
		} Type_;

		/** List of tracks in this release.
		 */
		QList<QList<ReleaseTrackInfo>> TrackInfos_;
	};

	/** @brief Pending discography request handle.
	 *
	 * Interface to a pending discography search in a
	 * IDiscographyProvider. A descendant of this class is returned from
	 * IDiscographyProvider::GetDiscography() or
	 * IDiscographyProvider::GetReleaseInfo(). In the former case
	 * GetReleases() contains all releases of the given artist, while in
	 * the latter — only those matching the requested release.
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
	 * @sa IDiscographyProvider
	 */
	class Q_DECL_EXPORT IPendingDisco
	{
	public:
		virtual ~IPendingDisco () {}

		/** @brief Returns this object as a QObject.
		 *
		 * This function can be used to connect to the signals of this
		 * class.
		 *
		 * @return This object as a QObject.
		 */
		virtual QObject* GetQObject () = 0;

		/** @brief Returns the list of found releases.
		 *
		 * This function returns the found releases, or an empty list if
		 * no releases are found, an error has occurred or search isn't
		 * completed yet..
		 *
		 * @return The fetched artist biography.
		 */
		virtual QList<ReleaseInfo> GetReleases () const = 0;
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
		 *
		 * @param[out] reason The human-readable string describing the
		 * error.
		 */
		virtual void error (const QString& reason) = 0;
	};

	/** @brief Interface for plugins supporting getting artist discography.
	 *
	 * Plugins that support fetching artists discography from sources
	 * like MusicBrainz should implement this interface.
	 *
	 * Discography includes various types of releases (albums, EPs,
	 * singles, etc) as well as the corresponding lists of tracks in
	 * those releases.
	 */
	class Q_DECL_EXPORT IDiscographyProvider
	{
	public:
		virtual ~IDiscographyProvider () {}

		/** @brief Returns the service name.
		 *
		 * This string returns a human-readable string with the service
		 * name, like "MusicBrainz".
		 *
		 * @return The human-readable service name.
		 */
		virtual QString GetServiceName () const = 0;

		/** @brief Fetches all the discography of the given artist.
		 *
		 * This function initiates a search for artist discography and
		 * returns a handle through which the results of the search could
		 * be obtained.
		 *
		 * All known releases of this artist are returned through the
		 * handle.
		 *
		 * The handle owns itself and deletes itself after results are
		 * available — see its documentation for details.
		 *
		 * @param[in] artist The artist name.
		 * @return The pending discography search handle.
		 */
		virtual IPendingDisco* GetDiscography (const QString& artist) = 0;

		/** @brief Fetches contents of the given release by the artist.
		 *
		 * This function initiates a search for the given release of the
		 * given artist and returns a handle through which the contents
		 * of the release can be obtained.
		 *
		 * Only matching release is returned through the handle, or no
		 * releases if, well, no releases match.
		 *
		 * The handle owns itself and deletes itself after results are
		 * available — see its documentation for details.
		 *
		 * @param[in] artist The artist name.
		 * @param[in] release The release name to search for.
		 * @return The pending discography search handle.
		 */
		virtual IPendingDisco* GetReleaseInfo (const QString& artist, const QString& release) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IPendingDisco, "org.LeechCraft.Media.IPendingDisco/1.0");
Q_DECLARE_INTERFACE (Media::IDiscographyProvider, "org.LeechCraft.Media.IDiscographyProvider/1.0");
