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

#include "separateplayer.h"
#include <QCloseEvent>
#include <QShortcut>

namespace LeechCraft
{
namespace Laure
{
	SeparatePlayer::SeparatePlayer (QWidget *parent)
	: QWidget (parent)
	, FullScreenMode_ (false)
	{
		setPalette (QPalette (Qt::black));
		InitShortcuts ();
	}

	void SeparatePlayer::InitShortcuts ()
	{
		new QShortcut (QKeySequence (Qt::Key_F), this, SLOT (close ()));
	}

	void SeparatePlayer::closeEvent (QCloseEvent *event)
	{
		emit closed ();
		event->accept ();
	}

	void SeparatePlayer::keyPressEvent (QKeyEvent *event)
	{
		switch (event->key ())
		{
		case Qt::Key_F11:
			FullScreenMode_ = !FullScreenMode_;
			if (FullScreenMode_)
				showFullScreen ();
			else
				showNormal ();
			break;
		}
	}
}
}