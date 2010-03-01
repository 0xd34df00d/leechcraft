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

#include "tabwidget.h"
#include <QTabBar>
#include <QHelpEvent>
#include <QAction>
#include <QMenu>
#include <QtDebug>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "tabbar.h"
#include "interfaces/imultitabs.h"
#include "3dparty/qxttooltip.h"

/**
 * Portions of this software derived from Qxt &copy; 2009, licensed
 * under the Common Public License, version 1.0, as published by IBM.
 * You should have received a copy of the CPL along with this software.
 */

using namespace LeechCraft;

TabWidget::TabWidget (QWidget *parent)
: QTabWidget (parent)
, AsResult_ (false)
{
	setTabBar (new TabBar (this));
	tabBar ()->setExpanding (false);
	tabBar ()->setContextMenuPolicy (Qt::CustomContextMenu);

	connect (tabBar (),
			SIGNAL (customContextMenuRequested (const QPoint&)),
			this,
			SLOT (handleTabBarContextMenu (const QPoint&)));
	connect (tabBar (),
			SIGNAL (tabMoved (int, int)),
			this,
			SLOT (handleMoveHappened (int, int)));
	connect (tabBar (),
			SIGNAL (tabMoved (int, int)),
			this,
			SIGNAL (moveHappened (int, int)));

	XmlSettingsManager::Instance ()->RegisterObject ("TabBarLocation",
			this, "handleTabBarLocationChanged");

	handleTabBarLocationChanged ();
}

void TabWidget::SetTooltip (int index, QWidget *widget)
{
	Widgets_ [index] = widget;
}

int TabWidget::TabAt (const QPoint& pos) const
{
	QPoint p = tabBar ()->mapFromGlobal (pos);
	return tabBar ()->tabAt (p);
}

void TabWidget::AddAction2TabBar (QAction *act)
{
	TabBarActions_ << act;
}

void TabWidget::InsertAction2TabBar (int index, QAction *act)
{
	TabBarActions_.insert(index, act);
}

bool TabWidget::event (QEvent *e)
{
	if (e->type () == QEvent::ToolTip)
	{
		QHelpEvent *he = static_cast<QHelpEvent*> (e);
		int index = tabBar ()->tabAt (he->pos ());
		if (Widgets_.contains (index) &&
				Widgets_ [index])
		{
			QxtToolTip::show (he->globalPos (), Widgets_ [index], tabBar ());
			return true;
		}
		else
			return false;
	}
	else
		return QTabWidget::event (e);
}

void TabWidget::mouseDoubleClickEvent (QMouseEvent *e)
{
	if (tabBar ()->tabAt (e->pos ()) == -1)
		emit newTabRequested ();

	QTabWidget::mouseDoubleClickEvent (e);
}

void TabWidget::tabRemoved (int index)
{
	Widgets_.remove (index);
	QList<int> keys = Widgets_.keys ();
	for (QList<int>::const_iterator i = keys.begin (),
			end = keys.end (); i != end; ++i)
		if (*i > index)
		{
			Widgets_ [*i - 1] = Widgets_ [*i];
			Widgets_.remove (*i);
		}
}

void TabWidget::handleTabBarLocationChanged ()
{
	QTabWidget::TabPosition pos = QTabWidget::North;

	QString value = XmlSettingsManager::Instance ()->
		property ("TabBarLocation").toString ();

	if (value == "south")
		pos = QTabWidget::South;
	else if (value == "west")
		pos = QTabWidget::West;
	else if (value == "east")
		pos = QTabWidget::South;

	setTabPosition (pos);
}

void TabWidget::handleTabBarContextMenu (const QPoint& pos)
{
	QMenu *menu = new QMenu ("", tabBar ());

	int tabIndex = tabBar ()->tabAt (pos);
	if (tabIndex != -1 &&
			XmlSettingsManager::Instance ()->
				property ("ShowPluginMenuInTabs").toBool ())
	{
		bool asSub = XmlSettingsManager::Instance ()->
			property ("ShowPluginMenuInTabsAsSubmenu").toBool ();
		IMultiTabsWidget *imtw =
			qobject_cast<IMultiTabsWidget*> (widget (tabIndex));
		if (imtw)
		{
			QList<QAction*> tabActions = imtw->GetTabBarContextMenuActions ();

			QMenu *subMenu = new QMenu (tabText (tabIndex), menu);
			Q_FOREACH (QAction *act, tabActions)
				(asSub ? subMenu : menu)->addAction (act);
			if (asSub)
				menu->addMenu (subMenu);
			if (tabActions.size ())
				menu->addSeparator ();
		}
	}

	Q_FOREACH (QAction *act, TabBarActions_)
	{
		act->setData (tabBar ()->mapToGlobal (pos));
		menu->addAction (act);
	}

	menu->exec (tabBar ()->mapToGlobal (pos));

	Q_FOREACH (QAction *act, TabBarActions_)
		act->setData (QVariant ());

	delete menu;
}

void TabWidget::handleMoveHappened (int from, int to)
{
	std::swap (Widgets_ [from], Widgets_ [to]);
}

