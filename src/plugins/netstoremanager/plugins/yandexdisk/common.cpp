/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "common.h"
#include <QIcon>

namespace LeechCraft
{
namespace NetStoreManager
{
namespace YandexDisk
{
	QIcon GetIconForType (QString type)
	{
		static QPixmap icons (":/netstoremanager/yandexdisk/resources/images/yandexnarod-icons-files.png");

		auto iconNum = [] (const QString& str)
		{
			if (str == "b-icon-music")
				return 0;
			else if (str == "b-icon-video")
				return 1;
			else if (str == "b-icon-arc")
				return 2;
			else if (str == "b-icon-doc")
				return 3;
			else if (str == "b-icon-soft")
				return 4;
			else if (str == "b-icon-picture")
				return 14;
			else
				return 5;
		};

		type.remove ("-old");

		return icons.copy (iconNum (type) * 16, 0, 16, 16);
	}
}
}
}
