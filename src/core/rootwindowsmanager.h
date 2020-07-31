/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include <interfaces/core/irootwindowsmanager.h>
#include "mainwindow.h"

class QIcon;
class QScreen;

namespace LC
{
	class TabManager;
	class MWProxy;

	class RootWindowsManager : public QObject
							 , public IRootWindowsManager
	{
		Q_OBJECT
		Q_INTERFACES (IRootWindowsManager)

		struct WinData
		{
			MainWindow *Window_;
			MWProxy *Proxy_;
			TabManager *TM_;
		};
		QList<WinData> Windows_;

		bool IsShuttingDown_ = false;
	public:
		RootWindowsManager (QObject* = nullptr);

		void Initialize ();
		void Release ();

		MainWindow* MakeMainWindow ();
		TabManager* GetTabManager (MainWindow*) const;
		TabManager* GetTabManager (int) const;

		bool WindowCloseRequested (MainWindow*);

		QObject* GetQObject () override;

		int GetWindowsCount () const override;
		int GetPreferredWindowIndex () const override;
		int GetPreferredWindowIndex (const ITabWidget*) const;
		int GetPreferredWindowIndex (const QByteArray&) const override;
		int GetWindowForTab (ITabWidget*) const override;
		int GetWindowIndex (QMainWindow*) const override;

		IMWProxy* GetMWProxy (int) const override;
		MainWindow* GetMainWindow (int) const override;
		ICoreTabWidget* GetTabWidget (int) const override;
	private:
		MainWindow* CreateWindow (QScreen *screen, bool primary);
		void PerformWithTab (const std::function<void (TabManager*, int)>&, QWidget*);
		void MoveTab (int tab, int fromWin, int toWin);
		void CloseWindowTabs (int index);
	public slots:
		void moveTabToNewWindow ();
		void moveTabToExistingWindow ();

		void add (const QString&, QWidget*);
		void remove (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void bringToFront (QWidget*);
	signals:
		void windowAdded (int) override;
		void windowRemoved (int) override;
		void currentWindowChanged (int, int) override;
		void tabAdded (int, int) override;
		void tabIsRemoving (int, int) override;
		void tabIsMoving (int, int, int) override;
		void tabMoved (int, int, int) override;

		void hookTabAdding (LC::IHookProxy_ptr, QWidget*);
		void hookGetPreferredWindowIndex (LC::IHookProxy_ptr, const QWidget*) const;
		void hookGetPreferredWindowIndex (LC::IHookProxy_ptr, const QByteArray&) const;
		void hookGetPreferredWindowIndex (LC::IHookProxy_ptr) const;
	};
}
