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

#include <QObject>
#include <QMap>
#include <QHash>
#include <QSet>
#include <interfaces/core/ihookproxy.h>

class QMainWindow;
class QDockWidget;
class QAction;

namespace LeechCraft
{
	class MainWindow;
	class RootWindowsManager;

	class DockManager : public QObject
	{
		Q_OBJECT

		RootWindowsManager *RootWM_;

		QMap<Qt::DockWidgetArea, QList<QDockWidget*>> Area2Widgets_;

		QHash<QDockWidget*, QWidget*> TabAssociations_;
		QHash<QAction*, QDockWidget*> ToggleAct2Dock_;
		QSet<QDockWidget*> ForcefullyClosed_;

		QHash<QDockWidget*, MainWindow*> Dock2Widnow_;
	public:
		DockManager (RootWindowsManager*, QObject* = 0);

		void AddDockWidget (QDockWidget*, Qt::DockWidgetArea);
		void AssociateDockWidget (QDockWidget*, QWidget*);

		void ToggleViewActionVisiblity (QDockWidget*, bool);
	protected:
		bool eventFilter (QObject*, QEvent*);
	private:
		void TabifyDW (QDockWidget*, Qt::DockWidgetArea);
	private slots:
		void handleDockDestroyed ();
		void handleDockLocationChanged (Qt::DockWidgetArea);
		void handleDockToggled (bool);
		void handleTabChanged (QWidget*);

		void handleWindow (int);
	signals:
		void hookDockWidgetActionVisToggled (LeechCraft::IHookProxy_ptr, QMainWindow*, QDockWidget*, bool);
	};
}
