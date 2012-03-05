/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef SEPARATETABBAR_H
#define SEPARATETABBAR_H

#include <QTabBar>
#include <QHash>

namespace LeechCraft
{
	class SeparateTabWidget;
	
	class SeparateTabBar : public QTabBar
	{
		Q_OBJECT
		int Id_;
		bool IsLastTab_;
		bool InMove_;
		SeparateTabWidget *TabWidget_;
	public:
		explicit SeparateTabBar (QWidget* = 0);
		void SetTabData (int);
		void SetTabClosable (int index, bool closable, QWidget *closeButton = 0);
		void SetLastTab (bool);
		void SetTabWidget (SeparateTabWidget*);

		QTabBar::ButtonPosition GetCloseButtonPosition ();

		void SetInMove (bool inMove);
	protected:
		QSize tabSizeHint (int) const;
		void mouseReleaseEvent (QMouseEvent*);
		void mousePressEvent (QMouseEvent *event);
		void mouseDoubleClickEvent (QMouseEvent*);
		void tabInserted (int);
		void tabRemoved (int);
		void paintEvent (QPaintEvent*);
	signals:
		void addDefaultTab ();
		void showAddTabButton (bool);
		void tabWasInserted (int);
		void tabWasRemoved (int);
		void releasedMouseAfterMove (int index);
	};
}

#endif // SEPARATETABBAR_H
