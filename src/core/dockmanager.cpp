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

#include "dockmanager.h"
#include <QDockWidget>
#include <QToolButton>
#include <QToolBar>
#include <util/defaulthookproxy.h>
#include <interfaces/ihavetabs.h>
#include "tabmanager.h"
#include "core.h"
#include "rootwindowsmanager.h"
#include "mainwindow.h"
#include "docktoolbarmanager.h"

namespace LeechCraft
{
	DockManager::DockManager (RootWindowsManager *rootWM, QObject *parent)
	: QObject (parent)
	, RootWM_ (rootWM)
	{
		for (int i = 0; i < RootWM_->GetWindowsCount (); ++i)
			handleWindow (i);

		connect (RootWM_,
				SIGNAL (windowAdded (int)),
				this,
				SLOT (handleWindow (int)));
	}

	void DockManager::AddDockWidget (QDockWidget *dw, Qt::DockWidgetArea area)
	{
		auto win = static_cast<MainWindow*> (RootWM_->GetPreferredWindow ());
		win->addDockWidget (area, dw);
		Dock2Window_ [dw] = win;

		connect (dw,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handleDockDestroyed ()));

		Window2DockToolbarMgr_ [win]->AddDock (dw, area);
	}

	void DockManager::AssociateDockWidget (QDockWidget *dock, QWidget *tab)
	{
		dock->installEventFilter (this);

		TabAssociations_ [dock] = tab;

		auto rootWM = Core::Instance ().GetRootWindowsManager ();
		const auto winIdx = rootWM->GetWindowForTab (qobject_cast<ITabWidget*> (tab));
		if (winIdx >= 0)
			handleTabChanged (rootWM->GetTabManager (winIdx)->GetCurrentWidget ());
		else
			dock->setVisible (false);

		auto toggleAct = dock->toggleViewAction ();
		ToggleAct2Dock_ [toggleAct] = dock;
		connect (toggleAct,
				SIGNAL (triggered (bool)),
				this,
				SLOT (handleDockToggled (bool)));
	}

	void DockManager::ToggleViewActionVisiblity (QDockWidget *widget, bool visible)
	{
		auto win = Dock2Window_ [widget];

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookDockWidgetActionVisToggled (proxy, win, widget, visible);
		if (proxy->IsCancelled ())
			return;

		/*
		// TODO
		QAction *act = widget->toggleViewAction ();
		if (!visible)
			MenuView_->removeAction (act);
		else
			MenuView_->insertAction (MenuView_->actions ().first (), act);
		*/
	}

	bool DockManager::eventFilter (QObject *obj, QEvent *event)
	{
		if (event->type () != QEvent::Close)
			return false;

		auto dock = qobject_cast<QDockWidget*> (obj);
		if (!dock)
			return false;

		ForcefullyClosed_ << dock;

		return false;
	}

	void DockManager::handleTabMove (int from, int to, int tab)
	{
		auto rootWM = Core::Instance ().GetRootWindowsManager ();

		auto fromWin = static_cast<MainWindow*> (rootWM->GetMainWindow (from));
		auto toWin = static_cast<MainWindow*> (rootWM->GetMainWindow (to));
		auto widget = fromWin->GetTabWidget ()->Widget (tab);

		for (auto i = TabAssociations_.begin (), end = TabAssociations_.end (); i != end; ++i)
			if (*i == widget)
			{
				auto dw = i.key ();
				Dock2Window_ [dw] = toWin;

				const auto area = fromWin->dockWidgetArea (dw);

				fromWin->removeDockWidget (dw);
				Window2DockToolbarMgr_ [fromWin]->RemoveDock (dw);
				toWin->addDockWidget (area, dw);
				Window2DockToolbarMgr_ [toWin]->AddDock (dw, area);
			}
	}

	void DockManager::handleDockDestroyed ()
	{
		auto dock = static_cast<QDockWidget*> (sender ());

		auto toggleAct = ToggleAct2Dock_.key (dock);
		Window2DockToolbarMgr_ [Dock2Window_ [dock]]->HandleDockDestroyed (dock, toggleAct);

		TabAssociations_.remove (dock);
		ToggleAct2Dock_.remove (toggleAct);
		ForcefullyClosed_.remove (dock);
		Dock2Window_.remove (dock);
	}

	void DockManager::handleDockToggled (bool isVisible)
	{
		auto dock = ToggleAct2Dock_ [static_cast<QAction*> (sender ())];
		if (!dock)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown toggler"
					<< sender ();
			return;
		}

		if (isVisible)
			ForcefullyClosed_.remove (dock);
		else
			ForcefullyClosed_ << dock;
	}

	void DockManager::handleTabChanged (QWidget *tabWidget)
	{
		auto thisWindowIdx = RootWM_->GetWindowForTab (qobject_cast<ITabWidget*> (tabWidget));
		auto thisWindow = static_cast<MainWindow*> (RootWM_->GetMainWindow (thisWindowIdx));
		auto toolbarMgr = Window2DockToolbarMgr_ [thisWindow];

		QList<QDockWidget*> toShow;
		for (auto dock : TabAssociations_.keys ())
		{
			auto otherWidget = TabAssociations_ [dock];
			auto otherWindow = RootWM_->GetWindowIndex (Dock2Window_ [dock]);
			if (otherWindow != thisWindowIdx)
				continue;

			if (otherWidget != tabWidget)
			{
				dock->setVisible (false);
				toolbarMgr->RemoveDock (dock);
			}
			else if (!ForcefullyClosed_.contains (dock))
				toShow << dock;
		}

		for (auto dock : toShow)
		{
			dock->setVisible (true);
			toolbarMgr->AddDock (dock, thisWindow->dockWidgetArea (dock));
		}
	}

	void DockManager::handleWindow (int index)
	{
		auto win = static_cast<MainWindow*> (RootWM_->GetMainWindow (index));
		Window2DockToolbarMgr_ [win] = new DockToolbarManager (win);

		connect (win,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handleWindowDestroyed ()));
	}

	void DockManager::handleWindowDestroyed ()
	{
		auto win = static_cast<MainWindow*> (sender ());
		Window2DockToolbarMgr_.remove (win);
	}
}
