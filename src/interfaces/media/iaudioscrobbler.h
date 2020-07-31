/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDateTime>
#include "audiostructs.h"

namespace Media
{
	/** @brief Interface for plugins that support scrobbling.
	 *
	 * Plugins supporting scrobbling listened tracks to services like
	 * Last.FM or Jamendo should implement this interface.
	 */
	class Q_DECL_EXPORT IAudioScrobbler
	{
	public:
		virtual ~IAudioScrobbler () {}

		/** @brief A backdated track - a track with its real playback
		 * time.
		 */
		using BackdatedTrack_t = QPair<Media::AudioInfo, QDateTime>;

		/** @brief A list of backdated tracks.
		 */
		using BackdatedTracks_t = QList<BackdatedTrack_t>;

		/** @brief A list of optional features a scrobbler may support.
		 *
		 * @sa HasFeature()
		 */
		enum class Feature
		{
			/** @brief Whether the scrobbler allows sending audiotracks
			 * marked by past timestamps.
			 *
			 * @sa SendBackdated()
			 */
			Backdating
		};

		/** @brief Queries whether a given \em feature is supported.
		 *
		 * @param[in] feature The feature to check.
		 * @return
		 */
		virtual bool SupportsFeature (Feature feature) const = 0;

		/** @brief Returns the service name.
		 *
		 * This string returns a human-readable string with the service
		 * name, like "Last.FM".
		 *
		 * @return The human-readable service name.
		 */
		virtual QString GetServiceName () const = 0;

		/** @brief Notifies the scrobbler that a new track is playing.
		 *
		 * This function should be called by an audio player when a new
		 * track is playing. The track data is contained in the audio
		 * parameter.
		 *
		 * This function should only be called when a track has been
		 * changed, or when the user has explicitly restarted track
		 * playback. This function should \em not be called after pausing
		 * and resuming playback.
		 *
		 * @param[in] audio The information about currently playing track.
		 */
		virtual void NowPlaying (const AudioInfo& audio) = 0;

		/** @brief Sends a \em list of backdated tracks.
		 *
		 * If a scrobbler implements this method, it's also worth making
		 * sure SupportsFeature() returns <code>true</code> for
		 * Feature::Backdating.
		 *
		 * @param[in] list The list of tracks to send.
		 */
		virtual void SendBackdated (const BackdatedTracks_t& list) = 0;

		/** @brief Notifies the scrobbler that playback is stopped.
		 *
		 * This function should be called when user stops the playback of
		 * the current track. This function should \em not be called on
		 * pausing.
		 */
		virtual void PlaybackStopped () = 0;

		/** @brief Notifies the scrobbler that user loves current track.
		 *
		 * This function should be called when user explicitly states
		 * that he loves the current track, that is, the track described
		 * by the last call to NowPlaying().
		 *
		 * @sa BanCurrentTrack()
		 */
		virtual void LoveCurrentTrack () = 0;

		/** @brief Notifies the scrobbler that user hates current track.
		 *
		 * This function should be called when user explicitly states
		 * that he hates the current track, that is, the track described
		 * by the last call to NowPlaying().
		 *
		 * @sa LoveCurrentTrack()
		 */
		virtual void BanCurrentTrack () = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IAudioScrobbler, "org.LeechCraft.Media.IAudioScrobbler/1.0")
