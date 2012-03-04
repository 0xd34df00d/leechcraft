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

#ifndef INTERFACES_CORE_ICORETABWIDGET_H
#define INTERFACES_CORE_ICORETABWIDGET_H

#include <QTabBar>
#include <QVariant>

class QObject;
class QWidget;
class QIcon;
class QMenu;

class ICoreTabWidget
{
public:
	virtual ~ICoreTabWidget () {}

	virtual QObject* GetObject () = 0;

	virtual int WidgetCount () const = 0;
	virtual QWidget* Widget (int) const = 0;

	virtual QList<QAction*> GetPermanentActions () const = 0;

	virtual QVariant TabData (int index) const = 0;
	virtual void SetTabData (int index, QVariant data) = 0;

	virtual QString TabText (int index) const = 0;
	virtual void SetTabText (int index, const QString& text) = 0;

	virtual QIcon TabIcon (int index) const = 0;

	virtual QWidget* TabButton (int index, QTabBar::ButtonPosition positioin) const = 0;
	virtual QTabBar::ButtonPosition GetCloseButtonPosition () const = 0;
	virtual void SetTabClosable (int index, bool closable, QWidget *closeButton = 0) = 0;

	virtual int CurrentIndex () const = 0;

	virtual void MoveTab (int from, int to) = 0;

	virtual void setCurrentIndex (int index) = 0;
	virtual void setCurrentWidget (QWidget *widget) = 0;

	virtual void tabWasInserted (int index) = 0;
};

Q_DECLARE_INTERFACE (ICoreTabWidget, "org.Deviant.LeechCraft.ICoreTabWidget/1.0");

#endif
