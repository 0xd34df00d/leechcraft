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

class ITabWidget;
class QMainWindow;
class ICoreTabWidget;
class IMWProxy;

class IRootWindowsManager
{
public:
	virtual ~IRootWindowsManager () {}

	virtual QObject* GetObject () = 0;

	virtual int GetWindowsCount () const = 0;

	virtual int GetPreferredWindowIndex () const = 0;

	virtual QMainWindow* GetPreferredWindow () const
	{
		return GetMainWindow (GetPreferredWindowIndex ());
	}

	virtual int GetWindowForTab (ITabWidget*) const = 0;

	virtual IMWProxy* GetMWProxy (int) const = 0;

	virtual QMainWindow* GetMainWindow (int) const = 0;
	virtual int GetWindowIndex (QMainWindow*) const = 0;

	virtual ICoreTabWidget* GetTabWidget (int) const = 0;
	virtual int GetTabWidgetIndex (ICoreTabWidget *ictw) const
	{
		for (int i = 0; i < GetWindowsCount (); ++i)
			if (GetTabWidget (i) == ictw)
				return i;

		return -1;
	}
protected:
	virtual void windowAdded (int) = 0;

	virtual void windowRemoved (int) = 0;

	virtual void currentWindowChanged (int, int) = 0;

	virtual void tabAdded (int windowId, int tabId) = 0;

	virtual void tabIsRemoving (int windowId, int tabId) = 0;

	virtual void tabIsMoving (int fromWin, int toWin, int tabId) = 0;
	virtual void tabMoved (int fromWin, int toWin, int tabId) = 0;
};

Q_DECLARE_INTERFACE (IRootWindowsManager, "org.LeechCraft.IRootWindowsManager/1.0");
