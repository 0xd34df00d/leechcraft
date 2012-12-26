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

#pragma once

#include <functional>
#include <QObject>
#include <interfaces/core/irootwindowsmanager.h>

class QIcon;

namespace LeechCraft
{
	class MainWindow;
	class TabManager;
	class MWProxy;

	class RootWindowsManager : public QObject
							 , public IRootWindowsManager
	{
		Q_OBJECT
		Q_INTERFACES (IRootWindowsManager);

		struct WinData
		{
			MainWindow *Window_;
			MWProxy *Proxy_;
			TabManager *TM_;
		};
		QList<WinData> Windows_;
	public:
		RootWindowsManager (QObject* = 0);

		void Release ();
		MainWindow* MakeMainWindow ();
		TabManager* GetTabManager (MainWindow*) const;
		TabManager* GetTabManager (int) const;

		bool WindowCloseRequested (MainWindow*);

		QObject* GetObject ();

		int GetWindowsCount () const;
		int GetPreferredWindowIndex () const;
		int GetWindowForTab (ITabWidget*) const;
		int GetWindowIndex (QMainWindow*) const;

		IMWProxy* GetMWProxy (int) const;
		QMainWindow* GetMainWindow (int) const;
		ICoreTabWidget* GetTabWidget (int) const;
	private:
		MainWindow* CreateWindow ();
		void PerformWithTab (std::function<void (TabManager*)>, QWidget*);
		void MoveTab (int tab, int fromWin, int toWin);
	public slots:
		void moveTabToNewWindow ();
		void moveTabToExistingWindow ();

		void add (const QString&, QWidget*);
		void remove (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void bringToFront (QWidget*);
	signals:
		void windowAdded (int);
		void windowRemoved (int);
		void currentWindowChanged (int, int);
		void tabMovedXWindows (int, int);
	};
}
