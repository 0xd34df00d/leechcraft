/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "widthiconprovider.h"
#include <QIcon>

namespace LC::Util
{
	WidthIconProvider::WidthIconProvider ()
	: QQuickImageProvider (Pixmap)
	{
	}

	QPixmap WidthIconProvider::requestPixmap (const QString& idStr, QSize *size, const QSize& requestedSize)
	{
		auto list = idStr.split ('/', Qt::SkipEmptyParts);
		if (list.isEmpty ())
			return QPixmap ();

		auto realSize = requestedSize;
		if (realSize.width () <= 0)
		{
			bool ok = false;
			const int width = list.last ().toDouble (&ok);
			realSize = width > 0 ? QSize (width, width) : QSize (32, 32);
			if (ok)
				list.removeLast ();
		}

		const auto& icon = GetIcon (list);

		if (size)
			*size = icon.actualSize (realSize);

		return icon.pixmap (realSize);
	}
}
