/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fswinwatcher.h"
#include <QX11Info>
#include <QMainWindow>
#include <X11/Xlib.h>
#include <interfaces/core/irootwindowsmanager.h>

namespace LC
{
namespace Kinotify
{
	FSWinWatcher::FSWinWatcher (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	{
	}

	namespace
	{
		std::optional<QSize> GetSize (Display *dpy, Window win)
		{
			XWindowAttributes windowattr;
			if (XGetWindowAttributes (dpy, win, &windowattr) == 0)
				return {};
			return QSize { windowattr.width, windowattr.height };
		}
	}

	bool FSWinWatcher::IsCurrentFS ()
	{
		auto display = QX11Info::display ();
		if (!display)
			return false;

		Window focusWin;
		int reverToReturn;
		XGetInputFocus (display, &focusWin, &reverToReturn);

		auto rootWM = Proxy_->GetRootWindowsManager ();
		for (int i = 0; i < rootWM->GetWindowsCount (); ++i)
			if (rootWM->GetMainWindow (i)->effectiveWinId () == focusWin)
				return false;

		const auto rootSize = GetSize (display, RootWindow (display, QX11Info::appScreen ()));
		const auto focusSize = GetSize (display, focusWin);
		return rootSize && focusSize && *rootSize == *focusSize;
	}
}
}
