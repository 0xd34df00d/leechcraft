/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Minh Ngo
 * Copyright (C) 2006-2012  Georg Rudoy
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
#include <QMenu>
#include "vlcwrapper.h"

namespace LeechCraft
{
namespace Laure
{
	/** @brief Provides a menu for choosing a playback mode.
	 * 
	 * @author Minh Ngo <nlminhtl@gmail.com>
	 */
	class PlaybackModeMenu : public QMenu
	{
		Q_OBJECT
		PlaybackMode PlaybackMode_;
	public:
		/** @brief Constructs a new PlaybackModeMenu class
		 * with the given parent.
		 */
		PlaybackModeMenu (QWidget* = 0);
		
		/** @brief Returns the playback mode.
		 * 
		 * @sa PlaybackMode
		 */
		PlaybackMode GetPlaybackMode () const;
	private slots:
		void handleMenuDefault ();
		void handleMenuRepeat ();
		void handleMenuLoop ();
	signals:
		/** @brief Is emitted when a playback mode is chosen.
		 * 
		 * @param[out] mode Playback mode.
		 * 
		 * @sa PlaybackMode
		 */
		void playbackModeChanged (PlaybackMode mode);
	};
}
}