/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "flagiconprovider.h"
#include <QIcon>
#include <util/sys/paths.h>

namespace LC
{
namespace KBSwitch
{
	FlagIconProvider::FlagIconProvider ()
	: QQuickImageProvider (Pixmap)
	{
	}

	QPixmap FlagIconProvider::requestPixmap (const QString& id, QSize *actual, const QSize& requested)
	{
		QPixmap px (Util::GetSysPath (Util::SysPath::Share, "global_icons/flags", id + ".png"));

		if (px.isNull ())
			px = QIcon::fromTheme ("preferences-desktop-keyboard").pixmap (requested);

		if (actual)
			*actual = px.size ();
		return px;
	}
}
}
