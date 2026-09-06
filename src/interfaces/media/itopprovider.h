/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QList>
#include <QUrl>
#include <util/sll/eitherfwd.h>
#include "audiostructs.h"

template<typename>
class QFuture;

namespace Media
{
	/** @brief Contains information about an artist in the top charts.
	 *
	 * @sa ITopProvider
	 */
	struct TopArtistInfo
	{
		/** @brief Basic information about the artist.
		 *
		 * Contains basic common information about the artist, like name,
		 * description and tags.
		 */
		ArtistInfo Info_;

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

	/** @brief Contains information about a track in the top charts.
	 *
	 * @sa ITopProvider
	 */
	struct TopTrackInfo
	{
		/** @brief Name of the track.
		 */
		QString TrackName_;

		/** @brief Address of the track page.
		 *
		 * This field is expected to contain the address of the track on
		 * the service this TopTrackInfo is got from, not the artist
		 * web site.
		 */
		QUrl TrackPage_;

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
		 * the service this TopTrackInfo is got from, not the artist
		 * web site.
		 */
		QUrl ArtistPage_;
	};

	/** @brief Interface for plugins that provide top charts.
	 */
	class Q_DECL_EXPORT ITopProvider
	{
	public:
		virtual ~ITopProvider () {}

		/** @brief The result of a top artists query.
		 *
		 * Either a string with a human-readable error text, or the list
		 * of the top artists.
		 */
		using TopArtistsResult_t = LC::Util::Either<QString, QList<TopArtistInfo>>;

		/** @brief The result of a top tracks query.
		 *
		 * Either a string with a human-readable error text, or the list
		 * of the top tracks.
		 */
		using TopTracksResult_t = LC::Util::Either<QString, QList<TopTrackInfo>>;

		/** @brief Returns the service name.
		 *
		 * This string returns a human-readable string with the service
		 * name, like "Last.FM".
		 *
		 * @return The human-readable service name.
		 */
		virtual QString GetServiceName () const = 0;

		/** @brief Requests the list of top artists.
		 *
		 * @return The future holding the query result.
		 */
		virtual QFuture<TopArtistsResult_t> RequestTopArtists () = 0;

		/** @brief Requests the list of top tracks.
		 *
		 * @return The future holding the query result.
		 */
		virtual QFuture<TopTracksResult_t> RequestTopTracks () = 0;
	};
}

Q_DECLARE_INTERFACE (Media::ITopProvider, "org.LeechCraft.Media.ITopProvider/1.0")
