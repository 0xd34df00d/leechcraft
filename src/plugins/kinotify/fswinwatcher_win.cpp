/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 * Copyright (C) 2012 Dimitriy Ryazantcev
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

#include "fswinwatcher.h"
#include <QMainWindow>
#include <windows.h>
#if (NTDDI_VERSION >= NTDDI_VISTA)
#include <shellapi.h>
#endif

namespace LeechCraft
{
namespace Kinotify
{
	FSWinWatcher::FSWinWatcher (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	{
	}

	bool FSWinWatcher::IsCurrentFS ()
	{
#if (NTDDI_VERSION >= NTDDI_VISTA)
		QUERY_USER_NOTIFICATION_STATE state;
		if (SHQueryUserNotificationState (&state) != S_OK)
			return false;
		return state != QUNS_ACCEPTS_NOTIFICATIONS;
#else
		HWND hWnd = GetForegroundWindow ();
		if (!hWnd)
			return false;

		HMONITOR monitor = MonitorFromWindow (hWnd, MONITOR_DEFAULTTONULL);
		if (!monitor)
			return false;

		MONITORINFO lpmi;
		lpmi.cbSize = sizeof (lpmi);
		if (!GetMonitorInfo (monitor, &lpmi))
			return false;

		RECT monitorRect = lpmi.rcMonitor;
		RECT windowRect;
		GetWindowRect (hWnd, &windowRect);
		return EqualRect (&windowRect, &monitorRect) &&
				Proxy_->GetMainWindow ()->effectiveWinId () != hWnd;
#endif
	}
}
}
