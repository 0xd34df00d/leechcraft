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

#include "rootwindowsmanager.h"
#include <iterator>
#include <algorithm>
#include "core.h"
#include "mainwindow.h"
#include "mwproxy.h"
#include "tabmanager.h"
#include "dockmanager.h"
#include <interfaces/ihavetabs.h>

namespace LeechCraft
{
	RootWindowsManager::RootWindowsManager (QObject *parent)
	: QObject (parent)
	{
	}

	void RootWindowsManager::Release ()
	{
		for (const auto& win : Windows_)
			win.Window_->handleQuit ();
	}

	MainWindow* RootWindowsManager::MakeMainWindow ()
	{
		return CreateWindow ();
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
		const auto& data = Windows_ [index];
		for (int i = data.TM_->GetWidgetCount () - 1; i >= 0; --i)
			qobject_cast<ITabWidget*> (data.TM_->GetWidget (i))->Remove ();

		emit windowRemoved (index);

		Windows_.removeAt (index);

		win->deleteLater ();

		return true;
	}

	QObject* RootWindowsManager::GetObject ()
	{
		return this;
	}

	int RootWindowsManager::GetWindowsCount () const
	{
		return Windows_.size ();
	}

	int RootWindowsManager::GetPreferredWindowIndex () const
	{
		const auto active = QApplication::activeWindow ();
		if (!active)
			return 0;

		for (int i = 0; i < GetWindowsCount (); ++i)
			if (Windows_ [i].Window_ == active)
				return i;

		return 0;
	}

	int RootWindowsManager::GetWindowForTab (ITabWidget *tab) const
	{
		for (int i = 0; i < GetWindowsCount (); ++i)
		{
			const auto tw = Windows_ [i].Window_->GetTabWidget ();
			if (tw->IndexOf (dynamic_cast<QWidget*> (tab)) >= 0)
				return i;
		}

		return -1;
	}

	int RootWindowsManager::GetWindowIndex (QMainWindow *w) const
	{
		auto pos = std::find_if (Windows_.begin (), Windows_.end (),
				[w] (decltype (Windows_.at (0)) item) { return item.Window_ == w; });
		return pos == Windows_.end () ? -1 : std::distance (Windows_.begin (), pos);
	}

	QMainWindow* RootWindowsManager::GetMainWindow (int index) const
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

	MainWindow* RootWindowsManager::CreateWindow ()
	{
		auto win = new MainWindow;
		auto proxy = new MWProxy (win);
		auto tm = new TabManager (win->GetTabWidget (), win, win->GetTabWidget ());

		connect (tm,
				SIGNAL (currentTabChanged (QWidget*)),
				Core::Instance ().GetDockManager (),
				SLOT (handleTabChanged (QWidget*)));

		Windows_.push_back ({ win, proxy, tm });

		emit windowAdded (Windows_.size () - 1);

		win->Init ();

		return win;
	}

	void RootWindowsManager::PerformWithTab (std::function<void (TabManager*)> f, QWidget *w)
	{
		const int idx = GetWindowForTab (qobject_cast<ITabWidget*> (w));
		if (idx < 0)
		{
			qWarning () << Q_FUNC_INFO
					<< "no window for tab"
					<< w;
			return;
		}

		f (Windows_ [idx].TM_);
	}

	void RootWindowsManager::add (const QString& name, QWidget *w)
	{
		const int winIdx = GetPreferredWindowIndex ();

		auto itw = qobject_cast<ITabWidget*> (w);
		const int oldWinIdx = GetWindowForTab (itw);
		if (oldWinIdx >= 0 && oldWinIdx != winIdx)
			Windows_ [oldWinIdx].TM_->remove (w);

		Windows_ [winIdx].TM_->add (name, w);
	}

	void RootWindowsManager::remove (QWidget *w)
	{
		PerformWithTab ([w] (TabManager *tm) { tm->remove (w); }, w);
	}

	void RootWindowsManager::changeTabName (QWidget *w, const QString& name)
	{
		PerformWithTab ([w, &name] (TabManager *tm) { tm->changeTabName (w, name); }, w);
	}

	void RootWindowsManager::changeTabIcon (QWidget *w, const QIcon& icon)
	{
		PerformWithTab ([w, &icon] (TabManager *tm) { tm->changeTabIcon (w, icon); }, w);
	}

	void RootWindowsManager::bringToFront (QWidget *w)
	{
		PerformWithTab ([w] (TabManager *tm) { tm->bringToFront (w); }, w);
	}
}
