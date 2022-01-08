/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "platform.h"
#include <QPixmap>
#include "types.h"

namespace LC::Auscrie
{
	QPixmap GetPixmap (Mode mode)
	{
		switch (mode)
		{
		case Mode::LCWindowOverlay:
			return GetLCWindowOverlay ();
		case Mode::LCWindow:
			return GetLCWindow ();
		case Mode::CurrentScreen:
		case Mode::WholeDesktop:
			return {};
		}
	}
}
