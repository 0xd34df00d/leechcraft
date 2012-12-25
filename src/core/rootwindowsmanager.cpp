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

#include "rootwindowsmanager.h"
#include "core.h"
#include "mainwindow.h"
#include "mwproxy.h"

namespace LeechCraft
{
	RootWindowsManager::RootWindowsManager (QObject *parent)
	: QObject (parent)
	, MWProxy_ (new MWProxy (this))
	{
	}

	QObject* RootWindowsManager::GetObject ()
	{
		return this;
	}

	int RootWindowsManager::GetWindowsCount () const
	{
		return 1;
	}

	int RootWindowsManager::GetPreferredWindowIndex () const
	{
		return 0;
	}

	int RootWindowsManager::GetWindowForTab (ITabWidget*) const
	{
		return 0;
	}

	int RootWindowsManager::GetWindowIndex (QMainWindow*) const
	{
		return 0;
	}

	QMainWindow* RootWindowsManager::GetMainWindow (int) const
	{
		return Core::Instance ().GetReallyMainWindow ();
	}

	IMWProxy* RootWindowsManager::GetMWProxy (int) const
	{
		return MWProxy_;
	}

	ICoreTabWidget* RootWindowsManager::GetTabWidget (int) const
	{
		return Core::Instance ().GetReallyMainWindow ()->GetTabWidget ();
	}
}
