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

#include <windows.h>
#include "fswinwatcher.h"

namespace LeechCraft
{
namespace Kinotify
{
	FSWinWatcher::FSWinWatcher (QObject *parent)
	: QObject (parent)
	{
	}

	bool FSWinWatcher::IsCurrentFS ()
	{
		HWND hWnd = GetForegroundWindow ();
		if (hWnd)
		{
			HMONITOR monitor = MonitorFromWindow (hWnd, MONITOR_DEFAULTTONULL);
			MONITORINFO lpmi;
			lpmi.cbSize = sizeof (lpmi);
			if (GetMonitorInfo (monitor, &lpmi))
			{
				RECT monitorRect = lpmi.rcMonitor;
				RECT windowRect;
				GetWindowRect (hWnd, &windowRect);
				if (EqualRect (&windowRect, &monitorRect))
					return true;
			}
		}
		return false;
	}
}
}
