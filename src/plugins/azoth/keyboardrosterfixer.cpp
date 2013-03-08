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

#include "keyboardrosterfixer.h"
#include <QKeyEvent>
#include <QApplication>
#include <QTreeView>

namespace LeechCraft
{
namespace Azoth
{
	KeyboardRosterFixer::KeyboardRosterFixer (QTreeView *view, QObject *parent)
	: QObject (parent)
	, View_ (view)
	, IsSearching_ (false)
	{
	}

	bool KeyboardRosterFixer::eventFilter (QObject*, QEvent *e)
	{
		if (e->type () != QEvent::KeyPress &&
			e->type () != QEvent::KeyRelease)
			return false;

		QKeyEvent *ke = static_cast<QKeyEvent*> (e);
		if (!IsSearching_)
		{
			switch (ke->key ())
			{
			case Qt::Key_Space:
			case Qt::Key_Right:
			case Qt::Key_Left:
				qApp->sendEvent (View_, e);
				return true;
			default:
				;
			}
		}

		switch (ke->key ())
		{
		case Qt::Key_Down:
		case Qt::Key_Up:
		case Qt::Key_PageDown:
		case Qt::Key_PageUp:
		case Qt::Key_Enter:
		case Qt::Key_Return:
			IsSearching_ = false;
			qApp->sendEvent (View_, e);
			return true;
		default:
			IsSearching_ = true;
			return false;
		}
	}
}
}
