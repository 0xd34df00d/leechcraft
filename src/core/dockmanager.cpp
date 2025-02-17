/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dockmanager.h"
#include <algorithm>
#include <QDockWidget>
#include <QToolBar>
#include <QTimer>
#include <QResizeEvent>
#include <util/xpc/defaulthookproxy.h>
#include <util/sll/qtutil.h>
#include <interfaces/ihavetabs.h>
#include "tabmanager.h"
#include "core.h"
#include "rootwindowsmanager.h"
#include "mainwindow.h"
#include "docktoolbarmanager.h"
#include "mainwindowmenumanager.h"
#include "xmlsettingsmanager.h"

namespace LC
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

	void DockManager::AddDockWidget (QDockWidget *dw, const IMWProxy::DockWidgetParams& params)
	{
		auto win = static_cast<MainWindow*> (RootWM_->GetPreferredWindow ());
		win->addDockWidget (params.Area_, dw);
		win->resizeDocks ({ dw }, { 1 }, Qt::Horizontal);		// https://bugreports.qt.io/browse/QTBUG-65592
		Dock2Info_ [dw].Window_ = win;
		Dock2Info_ [dw].SizeContext_ = params.SizeContext_;

		connect (dw,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handleDockDestroyed ()));

		Window2DockToolbarMgr_ [win]->AddDock (dw, params.Area_);

		dw->installEventFilter (this);

		SetupDockAction (dw);
		if (params.SizeContext_)
			SetupSizing (dw, *params.SizeContext_);
	}

	void DockManager::AssociateDockWidget (QDockWidget *dock, QWidget *tab)
	{
		Dock2Info_ [dock].Associated_ = tab;

		auto rootWM = Core::Instance ().GetRootWindowsManager ();
		const auto winIdx = rootWM->GetWindowForTab (qobject_cast<ITabWidget*> (tab));
		if (winIdx >= 0)
			handleTabChanged (rootWM->GetTabManager (winIdx)->GetCurrentWidget ());
		else
			dock->setVisible (false);
	}

	void DockManager::ToggleViewActionVisiblity (QDockWidget *widget, bool visible)
	{
		auto win = Dock2Info_ [widget].Window_;

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookDockWidgetActionVisToggled (proxy, win, widget, visible);
		if (proxy->IsCancelled ())
			return;

		const auto toggleView = widget->toggleViewAction ();
		const auto menu = win->GetMenuManager ()->GetSubMenu (MainWindowMenuManager::Role::View);
		if (visible)
			menu->addAction (toggleView);
		else
			menu->removeAction (toggleView);
	}

	void DockManager::SetDockWidgetVisibility (QDockWidget *dw, bool visible)
	{
		dw->setVisible (visible);
		HandleDockToggled (dw, visible);
	}

	QSet<QDockWidget*> DockManager::GetWindowDocks (const MainWindow *window) const
	{
		QSet<QDockWidget*> result;
		for (auto i = Dock2Info_.begin (); i != Dock2Info_.end (); ++i)
			if (i->Window_ == window)
				result << i.key ();
		return result;
	}

	void DockManager::MoveDock (QDockWidget *dw, MainWindow *fromWin, MainWindow *toWin)
	{
		Dock2Info_ [dw].Window_ = toWin;

		const auto area = fromWin->dockWidgetArea (dw);

		fromWin->removeDockWidget (dw);
		Window2DockToolbarMgr_ [fromWin]->RemoveDock (dw);
		toWin->addDockWidget (area, dw);
		Window2DockToolbarMgr_ [toWin]->AddDock (dw, area);
	}

	QSet<QDockWidget*> DockManager::GetForcefullyClosed () const
	{
		return ForcefullyClosed_;
	}

	bool DockManager::eventFilter (QObject *obj, QEvent *event)
	{
		auto dock = qobject_cast<QDockWidget*> (obj);
		if (!dock)
			return false;

		switch (event->type ())
		{
		case QEvent::Close:
			ForcefullyClosed_ << dock;
			break;
		case QEvent::Hide:
			Dock2Info_ [dock].Width_ = dock->width ();
			break;
		case QEvent::Resize:
			if (const auto ctx = Dock2Info_ [dock].SizeContext_)
			{
				auto resizeEv = static_cast<QResizeEvent *> (event);
				const auto width = resizeEv->size ().width ();
				XmlSettingsManager::Instance ()->setProperty (*ctx, width);
			}
			break;
		case QEvent::Show:
		{
			const auto width = Dock2Info_ [dock].Width_;
			if (width > 0)
			{
				const auto prevMin = dock->minimumWidth ();
				const auto prevMax = dock->maximumWidth ();

				dock->setMinimumWidth (width);
				dock->setMaximumWidth (width);

				QTimer::singleShot (0, this,
						[dock = QPointer { dock }, prevMin, prevMax]
						{
							if (!dock)
								return;

							dock->setMinimumWidth (prevMin);
							dock->setMaximumWidth (prevMax);
						});
			}
			break;
		}
		default:
			break;
		}

		return false;
	}

	void DockManager::SetupDockAction (QDockWidget *dw)
	{
		auto toggleAct = dw->toggleViewAction ();
		ToggleAct2Dock_ [toggleAct] = dw;
		connect (toggleAct,
				&QAction::triggered,
				this,
				[this, dw] (bool isVisible) { HandleDockToggled (dw, isVisible); });
	}

	void DockManager::SetupSizing (QDockWidget *dw, const QByteArray& sizingContext)
	{
		const auto& storedWidth = XmlSettingsManager::Instance ()->property (sizingContext);
		if (storedWidth.isValid ())
			Dock2Info_ [dw].Width_ = storedWidth.toInt ();
	}

	void DockManager::HandleDockToggled (QDockWidget *dock, bool isVisible)
	{
		if (isVisible)
		{
			if (ForcefullyClosed_.remove (dock))
			{
				auto win = Dock2Info_ [dock].Window_;
				Window2DockToolbarMgr_ [win]->AddDock (dock, win->dockWidgetArea (dock));
			}
		}
		else
			ForcefullyClosed_ << dock;
	}

	void DockManager::handleTabMove (int from, int to, int tab)
	{
		auto rootWM = Core::Instance ().GetRootWindowsManager ();

		auto fromWin = rootWM->GetMainWindow (from);
		auto toWin = rootWM->GetMainWindow (to);
		auto widget = fromWin->GetTabWidget ()->Widget (tab);

		for (auto i = Dock2Info_.begin (), end = Dock2Info_.end (); i != end; ++i)
			if (i->Associated_ == widget)
				MoveDock (i.key (), fromWin, toWin);
	}

	void DockManager::handleDockDestroyed ()
	{
		auto dock = static_cast<QDockWidget*> (sender ());

		auto toggleAct = ToggleAct2Dock_.key (dock);
		Window2DockToolbarMgr_ [Dock2Info_ [dock].Window_]->HandleDockDestroyed (dock, toggleAct);

		Dock2Info_.remove (dock);
		ToggleAct2Dock_.remove (toggleAct);
		ForcefullyClosed_.remove (dock);
	}

	void DockManager::handleTabChanged (QWidget *tabWidget)
	{
		const auto thisWindowIdx = RootWM_->GetWindowForTab (qobject_cast<ITabWidget*> (tabWidget));
		const auto thisWindow = RootWM_->GetMainWindow (thisWindowIdx);
		const auto toolbarMgr = Window2DockToolbarMgr_ [thisWindow];

		const auto addDock = [=] (QDockWidget *dock)
		{
			if (dock->isVisible ())
				return;

			dock->setVisible (true);
			if (!dock->isFloating ())
				toolbarMgr->AddDock (dock, thisWindow->dockWidgetArea (dock));
		};

		QList<QDockWidget*> toShowFirst;
		QList<QDockWidget*> toShowLast;
		for (const auto& [dock, info] : Dock2Info_.asKeyValueRange ())
		{
			const auto otherWindow = RootWM_->GetWindowIndex (info.Window_);
			if (otherWindow != thisWindowIdx)
				continue;

			const auto otherWidget = info.Associated_;
			if (otherWidget && otherWidget != tabWidget)
			{
				dock->setVisible (false);
				toolbarMgr->RemoveDock (dock);
				continue;
			}
			if (ForcefullyClosed_.contains (dock))
				continue;

			// show unassociated docks first, since associated ones take precedence
			// and shall cover the unassociated ones
			if (otherWidget)
				toShowLast << dock;
			else
				toShowFirst << dock;
		}

		std::ranges::for_each (toShowFirst, addDock);
		std::ranges::for_each (toShowLast, addDock);
	}

	void DockManager::handleWindow (int index)
	{
		auto win = RootWM_->GetMainWindow (index);
		Window2DockToolbarMgr_ [win] = new DockToolbarManager (win, this);
		connect (win,
				&QObject::destroyed,
				this,
				[this, win] { Window2DockToolbarMgr_.remove (win); });
	}
}
