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

#include "mwproxy.h"
#include <QDockWidget>
#include "core.h"
#include "mainwindow.h"

namespace LeechCraft
{
	MWProxy::MWProxy (QObject *parent)
	: QObject (parent)
	{
	}

	void MWProxy::AddDockWidget (Qt::DockWidgetArea area, QDockWidget *w)
	{
		Core::Instance ().GetReallyMainWindow ()->addDockWidget (area, w);
		ToggleViewActionVisiblity (w, true);
	}

	void MWProxy::ToggleViewActionVisiblity (QDockWidget *w, bool visible)
	{
		Core::Instance ().GetReallyMainWindow ()->ToggleViewActionVisiblity (w, visible);
	}

	void MWProxy::AddToolbar (QToolBar *bar, Qt::ToolBarArea area)
	{
		bar->setParent (Core::Instance ().GetReallyMainWindow ());
		Core::Instance ().GetReallyMainWindow ()->addToolBar (area, bar);
	}

	void MWProxy::AddSideWidget (QWidget *w, WidgetArea area)
	{
		MainWindow *mw = Core::Instance ().GetReallyMainWindow ();
		QHBoxLayout *lay = qobject_cast<QHBoxLayout*> (mw->centralWidget ()->layout ());

		switch (area)
		{
		case WALeft:
			lay->insertWidget (0, w, 0, Qt::AlignTop);
			break;
		case WARight:
			lay->addWidget (w, 0, Qt::AlignTop);
			break;
		case WABottom:
			qWarning () << Q_FUNC_INFO
					<< "not implemented yet";
			break;
		}
	}
}
