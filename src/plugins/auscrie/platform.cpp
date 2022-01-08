/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "platform.h"
#include <QPixmap>
#include <QScreen>
#include <QMainWindow>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>

namespace LC::Auscrie
{
	QPixmap GetLCWindow ()
	{
		auto rootWin = GetProxyHolder ()->GetRootWindowsManager ()->GetPreferredWindow ();
		QPixmap px { rootWin->size () };
		px.fill (Qt::transparent);
		rootWin->render (&px);
		return px;
	}

	QPixmap GetLCWindowOverlay ()
	{
		auto rootWin = GetProxyHolder ()->GetRootWindowsManager ()->GetPreferredWindow ();
		return rootWin->screen ()->grabWindow (rootWin->winId ());
	}
}
