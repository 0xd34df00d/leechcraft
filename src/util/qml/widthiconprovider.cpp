/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "widthiconprovider.h"
#include <QIcon>

namespace LeechCraft
{
namespace Util
{
	WidthIconProvider::WidthIconProvider ()
	: QDeclarativeImageProvider (Pixmap)
	{
	}

	QPixmap WidthIconProvider::requestPixmap (const QString& idStr, QSize *size, const QSize& requestedSize)
	{
		const auto& list = idStr.split ('/', QString::SkipEmptyParts);
		if (list.isEmpty ())
			return QPixmap ();

		auto realSize = requestedSize;
		if (realSize.width () <= 0)
		{
			const int width = list.last ().toDouble ();
			realSize = width > 0 ? QSize (width, width) : QSize (32, 32);
		}

		const auto& icon = GetIcon (list);

		if (size)
			*size = icon.actualSize (realSize);

		return icon.pixmap (realSize);
	}
}
}
