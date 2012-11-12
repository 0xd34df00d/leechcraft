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

#include "sysiconsprovider.h"
#include <QIcon>

namespace LeechCraft
{
namespace LMP
{
	SysIconProvider::SysIconProvider (ICoreProxy_ptr proxy)
	: QDeclarativeImageProvider (Pixmap)
	, Proxy_ (proxy)
	{
	}

	QPixmap SysIconProvider::requestPixmap (const QString& id, QSize *size, const QSize& requestedSize)
	{
		const auto& icon = Proxy_->GetIcon (id);

		const auto& getSize = requestedSize.width () > 2 && requestedSize.height () > 2 ?
				requestedSize :
				QSize (48, 48);
		if (size)
			*size = icon.actualSize (getSize);
		return icon.pixmap (getSize);
	}
}
}
