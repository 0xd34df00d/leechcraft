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
	/** @brief Interface for plugins that support scrobbling.
	 *
	 * Plugins supporting scrobbling listened tracks to services like
	 * Last.FM or Jamendo should implement this interface.
	 */
	class Q_DECL_EXPORT IAudioScrobbler
	{
	public:
		virtual ~IAudioScrobbler () {}

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

Q_DECLARE_INTERFACE (Media::IAudioScrobbler, "org.LeechCraft.Media.IAudioScrobbler/1.0");
