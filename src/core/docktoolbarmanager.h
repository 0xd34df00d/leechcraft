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
#include <interfaces/core/ihookproxy.h>

class QToolBar;
class QDockWidget;
class QAction;
class QMainWindow;

namespace LC
{
	class MainWindow;
	class DockManager;

	class DockToolbarManager : public QObject
	{
		Q_OBJECT

		MainWindow * const Win_;
		const DockManager * const DockManager_;

		struct AreaInfo
		{
			Qt::DockWidgetArea Area_;
			QToolBar *Bar_;
		};
		QMap<Qt::DockWidgetArea, AreaInfo> Area2Info_;
		QMap<QAction*, QDockWidget*> Action2Widget_;
	public:
		DockToolbarManager (MainWindow*, DockManager*);

		void AddDock (QDockWidget*, Qt::DockWidgetArea);
		void RemoveDock (QDockWidget*);
		void HandleDockDestroyed (QDockWidget*, QAction*);
	private:
		void UpdateActionGroup (const QAction*, bool);
	private slots:
		void updateDockLocation (Qt::DockWidgetArea);
		void handleDockFloating (bool);
		void handleActionToggled (bool);
	signals:
		void hookAddingDockAction (LC::IHookProxy_ptr, QMainWindow*, QAction*, Qt::DockWidgetArea);
		void hookRemovingDockAction (LC::IHookProxy_ptr, QMainWindow*, QAction*, Qt::DockWidgetArea);

		void hookDockBarWillBeShown (LC::IHookProxy_ptr, QMainWindow*, QToolBar*, Qt::DockWidgetArea);
	};
}
