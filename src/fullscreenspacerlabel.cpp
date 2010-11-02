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

#include "fullscreenspacerlabel.h"
#include <QMouseEvent>
#include <QtDebug>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "mainwindow.h"
#include "tabwidget.h"
/**
 * Portions of this software derived from Qxt &copy; 2009, licensed
 * under the Common Public License, version 1.0, as published by IBM.
 * You should have received a copy of the CPL along with this software.
 */

using namespace LeechCraft;

FullscreenSpacerLabel::FullscreenSpacerLabel (QWidget *parent)
: QLabel (parent)
{
	this->hide ();

	if (!hasMouseTracking ())
		setMouseTracking (true);
}

void FullscreenSpacerLabel::mouseMoveEvent (QMouseEvent *event)
{
	MainWindow *wnd = Core::Instance ().GetReallyMainWindow ();
	if (wnd->windowState () == Qt::WindowFullScreen)
	{
		QMenuBar *menu = wnd->findChild<QMenuBar*> ("MenuBar_");
		QToolBar *toolbar = wnd->findChild<QToolBar*> ("MainToolbar_");
		LeechCraft::TabWidget *tabwidget = wnd->GetTabWidget ();
		QToolBar *bar = Core::Instance ().GetToolBar (tabwidget->currentIndex ());

		bool asButton = XmlSettingsManager::Instance ()->property ("ShowMenuBarAsButton").toBool ();

		if (event->y () < 5)
		{
			if (asButton)
				menu->hide ();
			else if (menu->isHidden ())
				menu->show ();

			if (toolbar->isHidden ())
				toolbar->show ();
			if (bar && bar->isHidden ())
				bar->show ();
		}
		else
		{
			if (!menu->isHidden ())
				menu->hide ();
			if (!toolbar->isHidden ())
				toolbar->hide ();
			if (bar && !bar->isHidden ())
				bar->hide ();
		}
	}
}
