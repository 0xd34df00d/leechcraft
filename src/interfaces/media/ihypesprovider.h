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
#include <QList>
#include <QUrl>
#include "audiostructs.h"

namespace Media
{
	/** @brief Contains information about a hyped artist.
	 *
	 * This structure is used to describe additional information about a
	 * hyped artist, like playcount or change in popularity.
	 *
	 * @sa IHypesProvider
	 */
	struct HypedArtistInfo
	{
		/** @brief Basic information about the artist.
		 *
		 * Contains basic common information about the artist, like name,
		 * description and tags.
		 */
		ArtistInfo Info_;

		/** @brief Change of popularity in percents.
		 *
		 * The period of time is unspecified, different services may
		 * choose to use different measures.
		 *
		 * This may be 0 if percentage change is unknown.
		 */
		int PercentageChange_;

		/** @brief Play count.
		 *
		 * The period of time is unspecified, different services may
		 * choose to use different measures.
		 *
		 * This may be 0 if play count is unknown.
		 */
		int Playcount_;

		/** @brief Number of listeners.
		 *
		 * The period of time is unspecified, different services may
		 * choose to use different measures.
		 *
		 * This may be 0 if listeners count is unknown.
		 */
		int Listeners_;
	};

	/** @brief Contains information about a hyped track.
	 *
	 * This structure is used to describe additional information about a
	 * hyped track, like playcount or change in popularity.
	 *
	 * @sa IHypesProvider
	 */
	struct HypedTrackInfo
	{
		/** @brief Name of the track.
		 */
		QString TrackName_;

		/** @brief Address of the track page.
		 *
		 * This field is expected to contain the address of the track on
		 * the service this HypedTrackInfo is got from, not the artist
		 * web site.
		 */
		QUrl TrackPage_;

		/** @brief Change of popularity in percents.
		 *
		 * The period of time is unspecified, different services may
		 * choose to use different measures.
		 *
		 * This may be 0 if percentage change is unknown.
		 */
		int PercentageChange_;

		/** @brief Play count.
		 *
		 * The period of time is unspecified, different services may
		 * choose to use different measures.
		 *
		 * This may be 0 if play count is unknown.
		 */
		int Playcount_;

		/** @brief Number of listeners.
		 *
		 * The period of time is unspecified, different services may
		 * choose to use different measures.
		 *
		 * This may be 0 if listeners count is unknown.
		 */
		int Listeners_;

		/** @brief Duration of the track.
		 */
		int Duration_;

		/** @brief URL of thumb image of this track or performing artist.
		 */
		QUrl Image_;

		/** @brief Full size image of this track or performing artist.
		 */
		QUrl LargeImage_;

		/** @brief Name of the performer of this track.
		 */
		QString ArtistName_;

		/** @brief URL of the artist page.
		 *
		 * This field is expected to contain the address of the artist on
		 * the service this HypedTrackInfo is got from, not the artist
		 * web site.
		 */
		QUrl ArtistPage_;
	};

	/** @brief Interface for plugins that support fetching hypes.
	 *
	 * Hypes are either popular tracks and artists or those who gain
	 * a lot of popularity right now.
	 *
	 * Fetching hypes is asynchronous in nature, so one should request
	 * updating the hypes list via RequestHype() method for each hype
	 * type one is interesting in, and then listen to gotHypedArtists()
	 * and gotHypedTracks() signals correspondingly.
	 */
	class Q_DECL_EXPORT IHypesProvider
	{
	public:
		virtual ~IHypesProvider () {}

		/** @brief Returns the service name.
		 *
		 * This string returns a human-readable string with the service
		 * name, like "Last.FM".
		 *
		 * @return The human-readable service name.
		 */
		virtual QString GetServiceName () const = 0;

		/** @brief The type of the hype.
		 */
		enum class HypeType
		{
			/** @brief New artists rapidly growing in popularity.
			 */
			NewArtists,

			/** @brief New tracks rapidly growing in popularity.
			 */
			NewTracks,

			/** @brief Top artists.
			 */
			TopArtists,

			/** @brief Top tracks.
			 */
			TopTracks
		};

		/** @brief Returns whether the service supports the given hype type.
		 *
		 * @param[in] hype The hype type to query.
		 * @return Whether the service supports this hype type.
		 */
		virtual bool SupportsHype (HypeType hype) = 0;

		/** @brief Updates the list of hyped artists of the given type.
		 *
		 * @param[in] type The type of the hype to update.
		 */
		virtual void RequestHype (HypeType type) = 0;
	protected:
		/** @brief Emitted when the list of hyped artists is updated.
		 *
		 * This signal is emitted when the list of hyped artists is
		 * updated for the given hype type.
		 *
		 * @param[out] artists The list of the artists for the given
		 * hype type.
		 * @param[out] type The type of the hype.
		 */
		virtual void gotHypedArtists (const QList<HypedArtistInfo>& artists, HypeType type) = 0;

		/** @brief Emitted when the list of hyped tracks is updated.
		 *
		 * This signal is emitted when the list of hyped tracks is
		 * updated for the given hype type.
		 *
		 * @param[out] tracks The list of the tracks for the given
		 * hype type.
		 * @param[out] type The type of the hype.
		 */
		virtual void gotHypedTracks (const QList<HypedTrackInfo>& tracks, HypeType type) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IHypesProvider, "org.LeechCraft.Media.IHypesProvider/1.0");
