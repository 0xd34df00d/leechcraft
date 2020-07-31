/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "childactioneventfilter.h"
#include <QEvent>
#include <QAction>
#include <QPushButton>
#include <QTabWidget>
#include <QActionEvent>
#include <QToolButton>
#include <QMenu>
#include <QToolBar>
#include <QtDebug>
#include "iconthemeengine.h"

using namespace LC;

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
		QChildEvent *e = static_cast<QChildEvent*> (event);
		auto child = e->child ();
		child->installEventFilter (this);

		if (auto act = qobject_cast<QAction*> (child))
			IconThemeEngine::Instance ().UpdateIconset ({ act });
		else if (auto tb = qobject_cast<QToolButton*> (child))
		{
			if (auto act = tb->defaultAction ())
				IconThemeEngine::Instance ().UpdateIconset ({ act });
			if (auto menu = tb->menu ())
				IconThemeEngine::Instance ().UpdateIconset ({ menu->menuAction () });
		}
		else if (auto pb = qobject_cast<QPushButton*> (child))
			IconThemeEngine::Instance ().UpdateIconset ({ pb });
		else
		{
			if (auto tb = qobject_cast<QToolBar*> (child))
				IconThemeEngine::Instance ().UpdateIconset (tb->actions ());
			else
				IconThemeEngine::Instance ().UpdateIconset (child->findChildren<QAction*> ());
			IconThemeEngine::Instance ().UpdateIconset (child->findChildren<QPushButton*> ());
			IconThemeEngine::Instance ().UpdateIconset (child->findChildren<QToolButton*> ());

			for (auto w : child->findChildren<QWidget*> ())
				w->installEventFilter (this);
		}
		return false;
	}
	else
		return QObject::eventFilter (obj, event);
}

