/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <util/sll/eitherfwd.h>
#include "audiostructs.h"

class QObject;
class QIcon;

template<typename>
class QFuture;

namespace Media
{
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
		/** @brief The title of a track.
		 *
		 * At least this or Artist_ field should not be empty.
		 */
		QString Title_;

		/** @brief The artist performing the track.
		 *
		 * At least this or Title_ field should not be empty.
		 */
		QString Artist_;

		/** @brief The album containing this track.
		 */
		QString Album_;

		/** @brief The approximate length of the track.
		 *
		 * Set this to 0 to disable by-track filtering.
		 */
		int TrackLength_ = 0;

		/** @brief Free form engine-specific request.
		 *
		 * Calling plugins should set this instead of Title_ or Artist_
		 * fields if they are not sure what user has entered.
		 */
		QString FreeForm_;
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

		/** @brief A structure describing a single entry in search result.
		 */
		struct Result
		{
			/** @brief The information about the found audio track.
			 */
			AudioInfo Info_;

			/** @brief The URL of this audio track.
			 */
			QUrl Source_;
		};

		/** @brief A list of successful audio search results.
		 */
		using Results_t = QList<Result>;

		/** @brief The result of an audio search query.
		 *
		 * The result of an audio search query is either a string with a
		 * human-readable error text, or a list of result items.
		 *
		 * @sa Results_t
		 */
		using Result_t = LC::Util::Either<QString, Results_t>;

		/** @brief Returns the name of this service.
		 *
		 * This function returns the name of the service this IAudioPile
		 * represents, like "VKontakte".
		 *
		 * @return The well-known service name.
		 */
		virtual QString GetServiceName () const = 0;

		/** @brief Returns the icon of this service.
		 *
		 * This function returns the icon of the service this IAudioPile
		 * represents.
		 *
		 * @return The service icon.
		 */
		virtual QIcon GetServiceIcon () const = 0;

		/** @brief Requests a search by the given request.
		 *
		 * This function initiates a search by the given request and
		 * returns a future with the search results.
		 *
		 * @param[in] request The structure describing the search request.
		 * @return The future with the audio search results.
		 */
		virtual QFuture<Result_t> Search (const AudioSearchRequest& request) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IAudioPile, "org.LeechCraft.Media.IAudioPile/1.0")
