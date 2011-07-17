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
		QHash<int, QString> PinTabsIndex2Name_;
		QHash<int, QWidget*> PinTabsIndex2CloseWidget_;
		QTabBar::ButtonPosition CloseSide_;
		SeparateTabWidget *TabWidget_;
	public:
		explicit SeparateTabBar (QWidget* = 0);
		bool IsPinTab (int) const;
		int PinTabsCount () const;
		QList<int> GetPinTabs () const;
		void SetTabData (int);
		void SetTabNoClosable (int);
		void SetLastTab (bool);
		void SetTabWidget (SeparateTabWidget*);
		QString GetPinTabText (int) const;
	protected:
		QSize tabSizeHint (int) const;
		void mouseReleaseEvent (QMouseEvent*);
		void mousePressEvent (QMouseEvent *event);
		void mouseDoubleClickEvent (QMouseEvent*);
		void tabInserted (int);
		void tabRemoved (int);
		void paintEvent (QPaintEvent*);
	public slots:
		void setPinTab (int);
		void setUnPinTab (int);
	signals:
		void addDefaultTab (bool);
		void showAddTabButton (bool);
		void tabWasInserted (int);
		void tabWasRemoved (int);
		
	};
}
#endif // SEPARATETABBAR_H
