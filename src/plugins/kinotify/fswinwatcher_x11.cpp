/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fswinwatcher.h"
#include <optional>
#include <QMainWindow>
#include <util/x11/xwrapper.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>

#include <X11/Xlib.h>

namespace LC::Kinotify
{
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

	bool IsCurrentWindowFullScreen ()
	{
		auto& xwrapper = Util::XWrapper::Instance ();
		auto display = xwrapper.GetDisplay ();
		if (!display)
			return false;

		Window focusWin;
		int reverToReturn;
		XGetInputFocus (display, &focusWin, &reverToReturn);

		auto rootWM = GetProxyHolder ()->GetRootWindowsManager ();
		for (int i = 0; i < rootWM->GetWindowsCount (); ++i)
			if (rootWM->GetMainWindow (i)->effectiveWinId () == focusWin)
				return false;

		const auto rootSize = GetSize (display, xwrapper.GetRootWindow ());
		const auto focusSize = GetSize (display, focusWin);
		return rootSize && focusSize && *rootSize == *focusSize;
	}
}
