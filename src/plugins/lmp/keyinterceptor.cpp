/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "keyinterceptor.h"
#include <QKeyEvent>
#include "core.h"

using namespace LeechCraft::Plugins::LMP;

KeyInterceptor::KeyInterceptor (QObject *parent)
: QObject (parent)
{
}

KeyInterceptor::~KeyInterceptor ()
{
}

bool KeyInterceptor::eventFilter (QObject *obj, QEvent *e)
{
	if (e->type () == QEvent::KeyPress)
	{
		keyPressEvent (dynamic_cast<QKeyEvent*> (e));
		return true;
	}
	else if (e->type () == QEvent::KeyRelease)
	{
		keyReleaseEvent (dynamic_cast<QKeyEvent*> (e));
		return true;
	}
	return QObject::eventFilter (obj, e);
}

void KeyInterceptor::keyPressEvent (QKeyEvent *e)
{
	if (e->text () == "*")
		Core::Instance ().IncrementVolume ();
	else if (e->text () == "/")
		Core::Instance ().DecrementVolume ();
	else if (e->key () == Qt::Key_Right)
		Core::Instance ().Forward (Core::SkipLittle);
	else if (e->key () == Qt::Key_Left)
		Core::Instance ().Rewind (Core::SkipLittle);
	else if (e->key () == Qt::Key_Up)
		Core::Instance ().Forward (Core::SkipMedium);
	else if (e->key () == Qt::Key_Down)
		Core::Instance ().Rewind (Core::SkipMedium);
	else if (e->key () == Qt::Key_PageUp)
		Core::Instance ().Forward (Core::SkipALot);
	else if (e->key () == Qt::Key_PageDown)
		Core::Instance ().Rewind (Core::SkipALot);
}

void KeyInterceptor::keyReleaseEvent (QKeyEvent *e)
{
	if (e->key () == Qt::Key_Return ||
			e->key () == Qt::Key_Enter)
		Core::Instance ().ToggleFullScreen ();
	else if (e->key () == Qt::Key_Space)
		Core::Instance ().TogglePause ();
}

