/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "platform.h"
#include <QCursor>
#include <QGuiApplication>
#include <QPixmap>
#include <QScreen>
#include <QtDebug>
#include <util/x11/xwrapper.h>
#include "types.h"

namespace LC::Auscrie
{
	namespace
	{
		QPixmap GetRootScreen (const QRect& geom)
		{
			auto screen = QGuiApplication::screenAt (geom.topLeft ());
			if (!screen)
				screen = QGuiApplication::primaryScreen ();

			const auto root = Util::XWrapper::Instance ().GetRootWindow ();
			return geom.isValid () ?
					screen->grabWindow (root, geom.x (), geom.y (), geom.width (), geom.height ()):
					screen->grabWindow (root);
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

		qWarning () << "unknown mode"
				<< static_cast<int> (mode);
		return {};
	}
}
