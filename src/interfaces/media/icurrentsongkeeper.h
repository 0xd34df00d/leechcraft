/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

namespace Media
{
	struct AudioInfo;

	/** @brief Interface for plugins able to fetch current tune.
	 *
	 * Plugins that are able to fetch current tune from audio players,
	 * both internal to LeechCraft like LMP and external ones (via MPRIS
	 * for example) should implement this interface.
	 */
	class ICurrentSongKeeper
	{
	public:
		virtual ~ICurrentSongKeeper () {}

		/** @brief Returns the information about the current song.
		 *
		 * @return The information about the currently playing song.
		 */
		virtual AudioInfo GetCurrentSong () const = 0;
	protected:
		/** @brief Emitted when current song changes.
		 *
		 * This signal should be emitted when the currently played tune
		 * is changed.
		 *
		 * @param[out] newTune The new currently playing song.
		 */
		virtual void currentSongChanged (const AudioInfo& newTune) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::ICurrentSongKeeper, "org.LeechCraft.Media.ICurrentSongKeeper/1.0")
