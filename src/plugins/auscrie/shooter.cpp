/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "shooter.h"
#include <QCursor>
#include <QGuiApplication>
#include <QPixmap>
#include <QScreen>
#include <QMainWindow>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "types.h"

namespace LC::Auscrie
{
	namespace
	{
		QPixmap GetLCWindow ()
		{
			auto rootWin = GetProxyHolder ()->GetRootWindowsManager ()->GetPreferredWindow ();
			const auto dpr = rootWin->devicePixelRatioF ();
			QPixmap px { rootWin->size () * dpr };
			px.setDevicePixelRatio (dpr);
			px.fill (Qt::transparent);
			rootWin->render (&px);
			return px;
		}

		QPixmap GetLCWindowOverlay ()
		{
			auto rootWin = GetProxyHolder ()->GetRootWindowsManager ()->GetPreferredWindow ();
			return rootWin->screen ()->grabWindow (rootWin->winId ());
		}

		QPixmap GetRootScreen (const QRect& geom)
		{
			auto screen = QGuiApplication::screenAt (geom.topLeft ());
			if (!screen)
				screen = QGuiApplication::primaryScreen ();

			const auto& rect = geom.isValid () ? geom : screen->virtualGeometry ();
			return screen->grabWindow (0, rect.x (), rect.y (), rect.width (), rect.height ());
		}
	}

	QPixmap GetPixmap (Mode mode)
	{
		switch (mode)
		{
		case Mode::LCWindowOverlay:
			return GetLCWindowOverlay ();
		case Mode::LCWindow:
			return GetLCWindow ();
		case Mode::CurrentScreen:
		{
			const auto screen = QGuiApplication::screenAt (QCursor::pos ());
			return GetRootScreen (screen ? screen->geometry () : QRect {});
		}
		case Mode::WholeDesktop:
			return GetRootScreen ({});
		}

		qWarning () << "unknown mode" << static_cast<int> (mode);
		return {};
	}
}
