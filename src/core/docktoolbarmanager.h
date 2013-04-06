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

#pragma once

#include <QObject>
#include <QMap>
#include <interfaces/core/ihookproxy.h>

class QToolBar;
class QDockWidget;
class QAction;
class QMainWindow;

namespace LeechCraft
{
	class MainWindow;

	class DockToolbarManager : public QObject
	{
		Q_OBJECT

		MainWindow *Win_;

		struct AreaInfo
		{
			Qt::DockWidgetArea Area_;
			QToolBar *Bar_;
		};
		QMap<Qt::DockWidgetArea, AreaInfo> Area2Info_;
		QMap<QAction*, QDockWidget*> Action2Widget_;
	public:
		DockToolbarManager (MainWindow*);

		void AddDock (QDockWidget*, Qt::DockWidgetArea);
		void RemoveDock (QDockWidget*);
		void HandleDockDestroyed (QDockWidget*, QAction*);
	private:
		void UpdateActionGroup (QAction*, bool);
	private slots:
		void updateDockLocation (Qt::DockWidgetArea);
		void handleActionToggled (bool);
	signals:
		void hookAddingDockAction (LeechCraft::IHookProxy_ptr, QMainWindow*, QAction*, Qt::DockWidgetArea);
		void hookRemovingDockAction (LeechCraft::IHookProxy_ptr, QMainWindow*, QAction*, Qt::DockWidgetArea);

		void hookDockBarWillBeShown (LeechCraft::IHookProxy_ptr, QMainWindow*, QToolBar*, Qt::DockWidgetArea);
	};
}
