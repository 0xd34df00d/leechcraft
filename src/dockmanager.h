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

class QDockWidget;

namespace LeechCraft
{
	class MainWindow;

	class DockManager : public QObject
	{
		Q_OBJECT

		MainWindow *MW_;
		QMap<Qt::DockWidgetArea, QList<QDockWidget*>> Area2Widgets_;
	public:
		DockManager (MainWindow*, QObject* = 0);

		void AddDockWidget (QDockWidget*, Qt::DockWidgetArea);
	private:
		void UnmanageFrom (QDockWidget*, QWidget*);
		void ManageInto (QDockWidget*, QWidget*);
	private slots:
		void handleDockLocationChanged (Qt::DockWidgetArea);
	};
}
