/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
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

#include "dockmanager.h"
#include <QDockWidget>
#include <QToolButton>
#include "mainwindow.h"

namespace LeechCraft
{
	DockManager::DockManager (MainWindow *mw, QObject *parent)
	: QObject (parent)
	, MW_ (mw)
	{
		MW_->GetDockListWidget (Qt::LeftDockWidgetArea)->hide ();
		MW_->GetDockListWidget (Qt::RightDockWidgetArea)->hide ();
	}

	void DockManager::AddDockWidget (QDockWidget *dw, Qt::DockWidgetArea area)
	{
		MW_->addDockWidget (area, dw);
		connect (dw,
				SIGNAL (dockLocationChanged (Qt::DockWidgetArea)),
				this,
				SLOT (handleDockLocationChanged (Qt::DockWidgetArea)));

		ManageInto (dw, MW_->GetDockListWidget (area));
	}

	void DockManager::UnmanageFrom (QDockWidget *dw, QWidget *w)
	{
		if (!w)
			return;

		/*
		for (int i = 0; i < w->layout ()->count (); ++i)
		{
			QToolButton *but = qobject_cast<QToolButton*> (w->layout ()->widget ());
			if (but->defaultAction () == dw->toggleViewAction ())
			{
				but->deleteLater ();
				w->layout ()->removeItem (w->layout ()->itemAt (i));
			}
		}
		*/
	}

	void DockManager::ManageInto (QDockWidget *dw, QWidget *w)
	{
		if (!w)
			return;

		/*
		QToolButton *button = new QToolButton ();
		button->setDefaultAction (dw->toggleViewAction ());
		w->layout ()->addWidget (button);
		if (!w->isVisible ())
			w->show ();
		*/
	}

	void DockManager::handleDockLocationChanged (Qt::DockWidgetArea area)
	{
		auto dw = qobject_cast<QDockWidget*> (sender ());
		if (!dw)
			return;

		Qt::DockWidgetArea from = Qt::NoDockWidgetArea;
		Q_FOREACH (from, Area2Widgets_.keys ())
		{
			if (Area2Widgets_ [from].removeAll (dw))
				break;
			from = Qt::NoDockWidgetArea;
		}

		Area2Widgets_ [area] << dw;

		UnmanageFrom (dw, MW_->GetDockListWidget (from));
		ManageInto (dw, MW_->GetDockListWidget (area));
	}
}
