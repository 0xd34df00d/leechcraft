/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
 * Copyright (C) 2006-2011  Georg Rudoy
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
		
		actionDefault->setProperty ("ActionIcon", "default");
		actionLoop->setProperty ("ActionIcon", "loop");
		actionRepeat->setProperty ("ActionIcon", "repeat");

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
		setProperty ("ActionIcon", "default");
		emit playbackModeChanged (PlaybackModeDefault);
	}
	
	void PlaybackModeMenu::handleMenuLoop ()
	{
		setProperty ("ActionIcon", "loop");
		emit playbackModeChanged (PlaybackModeLoop);
	}
	
	void PlaybackModeMenu::handleMenuRepeat ()
	{
		setProperty ("ActionIcon", "repeat");
		emit playbackModeChanged (PlaybackModeRepeat);
	}
}
}