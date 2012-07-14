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

#include "playbackmodemenu.h"
#include <QAction>

namespace LeechCraft
{
namespace Laure
{
	PlaybackModeMenu::PlaybackModeMenu (QWidget *parent)
	: QMenu (parent)
	, PlaybackMode_ (PlaybackModeDefault)
	{
		QAction *actionDefault = new QAction (tr ("Default"), this);
		QAction *actionLoop = new QAction (tr ("Loop"), this);
		QAction *actionRepeat = new QAction (tr ("Repeat"), this);
		
		actionDefault->setProperty ("ActionIcon", "view-list-details");
		actionLoop->setProperty ("ActionIcon", "media-playlist-shuffle");
		actionRepeat->setProperty ("ActionIcon", "media-playlist-repeat");

		addAction (actionDefault);
		addAction (actionLoop);
		addAction (actionRepeat);
		
		setProperty ("WatchActionIconChange", true);
		
		connect (actionDefault,
				SIGNAL (triggered (bool)),
				this,
				SLOT (handleMenuDefault ()));
		connect (actionLoop,
				SIGNAL (triggered (bool)),
				this,
				SLOT (handleMenuLoop ()));
		connect (actionRepeat,
				SIGNAL (triggered (bool)),
				this,
				SLOT (handleMenuRepeat ()));
	}
	
	PlaybackMode PlaybackModeMenu::GetPlaybackMode () const
	{
		return PlaybackMode_;
	}
	
	void PlaybackModeMenu::handleMenuDefault ()
	{
		setProperty ("ActionIcon", "services");
		emit playbackModeChanged (PlaybackModeDefault);
	}
	
	void PlaybackModeMenu::handleMenuLoop ()
	{
		setProperty ("ActionIcon", "media-playlist-shuffle");
		emit playbackModeChanged (PlaybackModeLoop);
	}
	
	void PlaybackModeMenu::handleMenuRepeat ()
	{
		setProperty ("ActionIcon", "media-playlist-repeat");
		emit playbackModeChanged (PlaybackModeRepeat);
	}
}
}