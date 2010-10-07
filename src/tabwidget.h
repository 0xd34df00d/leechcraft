/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef TABWIDGET_H
#define TABWIDGET_H
#include <QTabWidget>
#include <QMap>

namespace LeechCraft
{
	class TabWidget : public QTabWidget
	{
		Q_OBJECT

		QMap<int, QWidget*> Widgets_;
		QList<QAction*> TabBarActions_;
	public:
		TabWidget (QWidget* = 0);
		void SetTooltip (int, QWidget*);
		int TabAt (const QPoint&) const;
		void AddAction2TabBar (QAction*);
		void InsertAction2TabBar (int, QAction*);
	protected:
		virtual bool event (QEvent*);
		virtual void mouseDoubleClickEvent (QMouseEvent*);
		virtual void mouseReleaseEvent (QMouseEvent*);
		virtual void tabRemoved (int);
		virtual void mouseMoveEvent (QMouseEvent*);
	private slots:
		void handleTabBarLocationChanged ();
		void handleTabBarContextMenu (const QPoint&);
		void handleMoveHappened (int, int);
	signals:
		void moveHappened (int, int);
		void newTabRequested ();
		void newTabMenuRequested ();
	};
};

#endif

