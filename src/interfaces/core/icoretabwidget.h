/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef INTERFACES_CORE_ICORETABWIDGET_H
#define INTERFACES_CORE_ICORETABWIDGET_H
#include <QTabBar>

class QObject;
class QWidget;
class QIcon;

class ICoreTabWidget
{
public:
	virtual ~ICoreTabWidget () {}

	virtual QObject* GetObject () = 0;
	virtual int WidgetCount () const = 0;
	virtual QWidget* Widget (int) const = 0;
	virtual void AddAction2TabBarLayout (QTabBar::ButtonPosition, QAction*) = 0;
	virtual int IndexOf (QWidget*) const = 0;
	virtual QIcon TabIcon (int) const = 0;
	virtual QString TabText (int) const = 0;
	virtual bool IsPinTab (int) const = 0;
	virtual int CurrentIndex () const = 0;

	virtual void setCurrentIndex (int) = 0;
	virtual void setCurrentWidget (QWidget*) = 0;
};

Q_DECLARE_INTERFACE (ICoreTabWidget, "org.Deviant.LeechCraft.ICoreTabWidget/1.0");

#endif
