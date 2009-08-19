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

#include "childactioneventfilter.h"
#include <QEvent>
#include <QAction>
#include <QTabWidget>
#include <QActionEvent>
#include <QtDebug>
#include "skinengine.h"

using namespace LeechCraft;

ChildActionEventFilter::ChildActionEventFilter (QObject *parent)
: QObject (parent)
{
}

ChildActionEventFilter::~ChildActionEventFilter ()
{
}

bool ChildActionEventFilter::eventFilter (QObject *obj, QEvent *event)
{
	if (event->type () == QEvent::ChildAdded ||
			event->type () == QEvent::ChildPolished)
	{
		QChildEvent *e = dynamic_cast<QChildEvent*> (event);
		e->child ()->installEventFilter (this);

		QAction *act = dynamic_cast<QAction*> (e->child ());
		if (act)
			SkinEngine::Instance ().UpdateIconSet (QList<QAction*> () << act);
		else
		{
			SkinEngine::Instance ()
				.UpdateIconSet (e->child ()->findChildren<QAction*> ());
			SkinEngine::Instance ()
				.UpdateIconSet (e->child ()->findChildren<QTabWidget*> ());
		}
		return false;
	}
	else
		return QObject::eventFilter (obj, event);
}

