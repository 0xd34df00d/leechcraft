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
	/** @brief Pending similar artists request handle.
	 *
	 * Interface to similar artists search (and other, eh, similar
	 * searches like recommended artists request). A descendant of this
	 * class is returned from ISimilarArtists::GetSimilarArtists() and
	 * IRecommendedArtists::RequestRecommended() methods.
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
	 * @sa ISimilarArtists, IRecommendedArtists
	 */
	class Q_DECL_EXPORT IPendingSimilarArtists
	{
	public:
		virtual ~IPendingSimilarArtists () {}

		/** @brief Returns this object as a QObject.
		 *
		 * This function can be used to connect to the signals of this
		 * class.
		 *
		 * @return This object as a QObject.
		 */
		virtual QObject* GetQObject () = 0;

		/** @brief Returns the artist for which others are being searched.
		 *
		 * For some requests this doesn't make sense (like for the
		 * recommended artists), so this method returns a null string in
		 * these cases.
		 *
		 * @return The name of the artist for which other artists are
		 * searched, if applicable.
		 */
		virtual QString GetSourceArtistName () const = 0;

		/** @brief Returns the list of similar artists.
		 *
		 * This function returns the fetched list of similar artists, or
		 * an empty list if search is not complete or an error occured.
		 *
		 * @return The list of similar artists.
		 */
		virtual SimilarityInfos_t GetSimilar () const = 0;
	protected:
		/** @brief Emitted when the list is ready and fetched.
		 *
		 * The object will be invalid after this signal is emitted and
		 * the event loop is run.
		 */
		virtual void ready () = 0;

		/** @brief Emitted when there is an error fetching the list.
		 *
		 * The object will be invalid after this signal is emitted and
		 * the event loop is run.
		 */
		virtual void error () = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IPendingSimilarArtists, "org.LeechCraft.Media.IPendingSimilarArtists/1.0");

