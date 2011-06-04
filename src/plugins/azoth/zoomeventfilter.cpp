/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
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

#include "zoomeventfilter.h"
#include <QWheelEvent>
#include "chattab.h"

namespace LeechCraft
{
namespace Azoth
{
	ZoomEventFilter::ZoomEventFilter (QObject *parent)
	: QObject (parent)
	{
	}
	
	bool ZoomEventFilter::eventFilter (QObject *viewObj, QEvent *someEvent)
	{
		if (someEvent->type () != QEvent::Wheel)
			return false;
		
		QWheelEvent *e = static_cast<QWheelEvent*> (someEvent);
		if (!(e->modifiers () & Qt::ControlModifier))
			return false;

		int degrees = e->delta () / 8;
		qreal delta = static_cast<qreal> (degrees) / 150;
		QWebView *view = qobject_cast<QWebView*> (viewObj);
		view->setZoomFactor (view->zoomFactor () + delta);
		return true;
	}
}
}
