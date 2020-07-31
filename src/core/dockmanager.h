/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMap>
#include <QHash>
#include <QSet>
#include <QPointer>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/imwproxy.h>

class QMainWindow;
class QDockWidget;
class QAction;

namespace LC
{
	class MainWindow;
	class RootWindowsManager;
	class DockToolbarManager;

	class DockManager : public QObject
	{
		Q_OBJECT

		RootWindowsManager *RootWM_;

		struct DockInfo
		{
			QWidget *Associated_ = nullptr;
			MainWindow *Window_ = nullptr;
			int Width_ = -1;

			std::optional<QByteArray> SizeContext_ = {};
		};
		QHash<QDockWidget*, DockInfo> Dock2Info_;
		QHash<QAction*, QDockWidget*> ToggleAct2Dock_;
		QSet<QDockWidget*> ForcefullyClosed_;

		QHash<QMainWindow*, DockToolbarManager*> Window2DockToolbarMgr_;
	public:
		DockManager (RootWindowsManager*, QObject* = 0);

		void AddDockWidget (QDockWidget*, const IMWProxy::DockWidgetParams&);
		void AssociateDockWidget (QDockWidget*, QWidget*);

		void ToggleViewActionVisiblity (QDockWidget*, bool);

		void SetDockWidgetVisibility (QDockWidget*, bool);

		QSet<QDockWidget*> GetWindowDocks (const MainWindow*) const;
		void MoveDock (QDockWidget *dock, MainWindow *from, MainWindow *to);

		QSet<QDockWidget*> GetForcefullyClosed () const;
	protected:
		bool eventFilter (QObject*, QEvent*);
	private:
		void SetupDockAction (QDockWidget*);
		void SetupSizing (QDockWidget*, const QByteArray&);
		void HandleDockToggled (QDockWidget*, bool);
	public slots:
		void handleTabMove (int, int, int);
	private slots:
		void handleDockDestroyed ();
		void handleTabChanged (QWidget*);

		void handleWindow (int);
	signals:
		void hookDockWidgetActionVisToggled (LC::IHookProxy_ptr, QMainWindow*, QDockWidget*, bool);
	};
}
