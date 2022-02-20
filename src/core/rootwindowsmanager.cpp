/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "rootwindowsmanager.h"
#include <iterator>
#include <algorithm>
#include <QGuiApplication>
#include <util/xpc/defaulthookproxy.h>
#include <interfaces/ihavetabs.h>
#include "core.h"
#include "mainwindow.h"
#include "mwproxy.h"
#include "tabmanager.h"
#include "dockmanager.h"
#include "xmlsettingsmanager.h"
#include "application.h"

#if defined (Q_OS_UNIX) && defined (HAVE_X11)
#include <X11/Xutil.h>
#include <QX11Info>
#endif

namespace LC
{
	RootWindowsManager::RootWindowsManager (QObject *parent)
	: QObject (parent)
	{
	}

	void RootWindowsManager::Initialize ()
	{
		const bool deskMode = Application::instance ()->arguments ().contains ("--desktop");
		if (!deskMode)
		{
			CreateWindow (QGuiApplication::primaryScreen (), true);
			return;
		}

		for (const auto screen : QGuiApplication::screens ())
			CreateWindow (screen, true);
	}

	void RootWindowsManager::Release ()
	{
		for (const auto& winData : Windows_)
		{
			const auto win = winData.Window_;
			CloseWindowTabs (GetWindowIndex (win));
			win->handleQuit ();
		}

		IsShuttingDown_ = true;
	}

	MainWindow* RootWindowsManager::MakeMainWindow ()
	{
		const auto win = CreateWindow (QGuiApplication::primaryScreen (), false);
		win->show ();
		return win;
	}

	TabManager* RootWindowsManager::GetTabManager (MainWindow *win) const
	{
		return GetTabManager (GetWindowIndex (win));
	}

	TabManager* RootWindowsManager::GetTabManager (int index) const
	{
		return Windows_.value (index).TM_;
	}

	bool RootWindowsManager::WindowCloseRequested (MainWindow *win)
	{
		if (Windows_ [0].Window_ == win)
			return false;

		const int index = GetWindowIndex (win);
		CloseWindowTabs (index);

		auto mainWindow = Windows_ [0].Window_;

		auto dm = Core::Instance ().GetDockManager ();
		for (const auto& dock : dm->GetWindowDocks (win))
			dm->MoveDock (dock, win, mainWindow);

		emit windowRemoved (index);

		Windows_.removeAt (index);

		win->deleteLater ();

		return true;
	}

	QObject* RootWindowsManager::GetQObject ()
	{
		return this;
	}

	int RootWindowsManager::GetWindowsCount () const
	{
		return Windows_.size ();
	}

	int RootWindowsManager::GetPreferredWindowIndex () const
	{
		const auto& hookProxy = std::make_shared<Util::DefaultHookProxy> ();
		emit hookGetPreferredWindowIndex (hookProxy);
		if (hookProxy->IsCancelled ())
			return hookProxy->GetReturnValue ().toInt ();

		const auto active = QApplication::activeWindow ();
		if (!active)
			return 0;

		for (int i = 0; i < GetWindowsCount (); ++i)
			if (Windows_ [i].Window_ == active)
				return i;

		return 0;
	}

	int RootWindowsManager::GetPreferredWindowIndex (const ITabWidget *itw) const
	{
		const auto widget = dynamic_cast<const QWidget*> (itw);

		const auto& hookProxy = std::make_shared<Util::DefaultHookProxy> ();
		emit hookGetPreferredWindowIndex (hookProxy, widget);
		if (hookProxy->IsCancelled ())
			return hookProxy->GetReturnValue ().toInt ();

		return GetPreferredWindowIndex (itw->GetTabClassInfo ().TabClass_);
	}

	int RootWindowsManager::GetPreferredWindowIndex (const QByteArray& tc) const
	{
		const auto& hookProxy = std::make_shared<Util::DefaultHookProxy> ();
		emit hookGetPreferredWindowIndex (hookProxy, tc);
		if (hookProxy->IsCancelled ())
			return hookProxy->GetReturnValue ().toInt ();

		const auto& winMode = XmlSettingsManager::Instance ()->
				property ("WindowSelectionMode").toString ();
		if (winMode == "current")
			return GetPreferredWindowIndex ();

		QPair<int, int> currentMax { -1, 0 };
		for (int i = 0; i < GetWindowsCount (); ++i)
		{
			const auto tm = Windows_ [i].TM_;

			int count = 0;
			const auto widgetCount = tm->GetWidgetCount ();
			if (!widgetCount)
				return i;

			for (int j = 0; j < widgetCount; ++j)
			{
				auto other = qobject_cast<ITabWidget*> (tm->GetWidget (j));
				if (other->GetTabClassInfo ().TabClass_ == tc)
					++count;
			}

			if (count > currentMax.second)
				currentMax = { i, count };
		}

		return currentMax.first;
	}

	int RootWindowsManager::GetWindowForTab (ITabWidget *tab) const
	{
		auto tabWidget = dynamic_cast<QWidget*> (tab);
		for (int i = 0; i < GetWindowsCount (); ++i)
		{
			const auto tw = Windows_ [i].Window_->GetTabWidget ();
			if (tw->IndexOf (tabWidget) >= 0)
				return i;
		}

		return -1;
	}

	int RootWindowsManager::GetWindowIndex (QMainWindow *w) const
	{
		auto pos = std::find_if (Windows_.begin (), Windows_.end (),
				[w] (const auto& item) { return item.Window_ == w; });
		return pos == Windows_.end () ? -1 : std::distance (Windows_.begin (), pos);
	}

	MainWindow* RootWindowsManager::GetMainWindow (int index) const
	{
		return Windows_ [index].Window_;
	}

	IMWProxy* RootWindowsManager::GetMWProxy (int index) const
	{
		return Windows_ [index].Proxy_;
	}

	ICoreTabWidget* RootWindowsManager::GetTabWidget (int index) const
	{
		return Windows_ [index].Window_->GetTabWidget ();
	}

	MainWindow* RootWindowsManager::CreateWindow (QScreen *screen, bool primary)
	{
		const auto nextIdx = Windows_.size ();

		auto win = new MainWindow (screen, primary, nextIdx);
		auto proxy = new MWProxy (win);
		auto tm = new TabManager (win->GetTabWidget (), win, win->GetTabWidget ());

		connect (tm,
				SIGNAL (currentTabChanged (QWidget*)),
				Core::Instance ().GetDockManager (),
				SLOT (handleTabChanged (QWidget*)));

		Windows_.push_back ({ win, proxy, tm });
		win->Init ();

		emit windowAdded (nextIdx);

		return win;
	}

	template<typename F>
	void RootWindowsManager::PerformWithSender (F&& f)
	{
		if (IsShuttingDown_)
			return;

		const auto w = qobject_cast<QWidget*> (sender ());

		const int idx = GetWindowForTab (qobject_cast<ITabWidget*> (w));
		if (idx < 0)
		{
			qWarning () << Q_FUNC_INFO
					<< "no window for tab"
					<< w;
			return;
		}

		f (Windows_ [idx].TM_, w, idx);
	}

	void RootWindowsManager::MoveTab (int tabIdx, int fromWin, int toWin)
	{
		auto widget = Windows_ [fromWin].TM_->GetWidget (tabIdx);

		const auto sourceTW = Windows_ [fromWin].Window_->GetTabWidget ();
		const auto& name = sourceTW->TabText (tabIdx);
		const auto& icon = sourceTW->TabIcon (tabIdx);

		emit tabIsMoving (fromWin, toWin, tabIdx);

		auto targetMgr = Windows_ [toWin].TM_;
		Windows_ [fromWin].TM_->remove (widget);
		targetMgr->add (name, widget);
		targetMgr->changeTabIcon (widget, icon);

		emit tabMoved (fromWin, toWin, Windows_ [toWin].TM_->FindTabForWidget (widget));
	}

	void RootWindowsManager::CloseWindowTabs (int index)
	{
		const auto& data = Windows_ [index];
		for (int i = data.TM_->GetWidgetCount () - 1; i >= 0; --i)
			qobject_cast<ITabWidget*> (data.TM_->GetWidget (i))->Remove ();
	}

	void RootWindowsManager::moveTabToNewWindow ()
	{
		MakeMainWindow ();

		MoveTab (sender ()->property ("TabIndex").toInt (),
				sender ()->property ("FromWindowIndex").toInt (),
				GetWindowsCount () - 1);
	}

	void RootWindowsManager::moveTabToExistingWindow ()
	{
		MoveTab (sender ()->property ("TabIndex").toInt (),
				sender ()->property ("FromWindowIndex").toInt (),
				sender ()->property ("ToWindowIndex").toInt ());
	}

	namespace
	{
#if defined (Q_OS_UNIX) && defined (HAVE_X11)
		void SetWMClass (QWidget *w, QByteArray name)
		{
			XClassHint hint;
			hint.res_class = name.data ();
			hint.res_name = strdup ("leechcraft");
			XSetClassHint (QX11Info::display (), w->winId (), &hint);
			free (hint.res_name);
		}
#else
		void SetWMClass (QWidget*, QByteArray)
		{
		}
#endif
	}

	void RootWindowsManager::AddTab (const QString& name, QWidget *w, AddTabFlags flags)
	{
		auto raiseIfNeeded = [=] (int idx)
		{
			if (!(flags & AddTabFlag::Background))
				Windows_ [idx].TM_->bringToFront (w);
		};

		auto itw = qobject_cast<ITabWidget*> (w);

		if (const auto idx = GetWindowForTab (itw);
			idx != -1)
		{
			raiseIfNeeded (idx);
			return;
		}

		const auto& hookProxy = std::make_shared<Util::DefaultHookProxy> ();
		emit hookTabAdding (hookProxy, w);
		if (hookProxy->IsCancelled ())
			return;

		int winIdx = GetPreferredWindowIndex (itw);

		const int oldWinIdx = GetWindowForTab (itw);
		if (oldWinIdx >= 0 && oldWinIdx != winIdx)
		{
			const auto& oldData = Windows_ [oldWinIdx];
			emit tabIsRemoving (winIdx, oldData.Window_->GetTabWidget ()->IndexOf (w));
			oldData.TM_->remove (w);
		}

		if (winIdx == -1)
		{
			MakeMainWindow ();
			winIdx = Windows_.size () - 1;
		}

		while (winIdx >= Windows_.size ())
			MakeMainWindow ();

		auto& window = Windows_ [winIdx];

		const auto& tc = itw->GetTabClassInfo ().TabClass_;
		window.Window_->setWindowRole (tc);
		SetWMClass (window.Window_, tc);
		window.TM_->add (name, w);
		emit tabAdded (winIdx, window.Window_->GetTabWidget ()->IndexOf (w));

		ConnectSignals (w);

		raiseIfNeeded (winIdx);
	}

	void RootWindowsManager::ConnectSignals (QWidget *w)
	{
		connect (w,
				SIGNAL (removeTab ()),
				this,
				SLOT (remove ()));

		const auto mo = w->metaObject ();
		const auto hasSignal = [mo] (const char *signal)
		{
			return mo->indexOfSignal (QMetaObject::normalizedSignature (signal)) >= 0;
		};
		if (hasSignal ("changeTabName (QString)"))
			connect (w,
					SIGNAL (changeTabName (QString)),
					this,
					SLOT (changeTabName (QString)));
		if (hasSignal ("changeTabIcon (QIcon)"))
			connect (w,
					SIGNAL (changeTabIcon (QIcon)),
					this,
					SLOT (changeTabIcon (QIcon)));
		if (hasSignal ("raiseTab ()"))
			connect (w,
					SIGNAL (raiseTab ()),
					this,
					SLOT (bringToFront ()));
	}

	void RootWindowsManager::remove ()
	{
		PerformWithSender ([this] (TabManager *tm, QWidget *w, int winIdx)
			{
				emit tabIsRemoving (winIdx, tm->FindTabForWidget (w));
				tm->remove (w);
			});
	}

	void RootWindowsManager::changeTabName (const QString& name)
	{
		PerformWithSender ([&name] (TabManager *tm, QWidget *w, int) { tm->changeTabName (w, name); });
	}

	void RootWindowsManager::changeTabIcon (const QIcon& icon)
	{
		PerformWithSender ([&icon] (TabManager *tm, QWidget *w, int) { tm->changeTabIcon (w, icon); });
	}

	void RootWindowsManager::bringToFront ()
	{
		PerformWithSender ([] (TabManager *tm, QWidget *w, int) { tm->bringToFront (w); });
	}
}
