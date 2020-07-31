/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemimageprovider.h"
#include <QUrl>
#include <util/xdg/item.h>

namespace LC
{
namespace Launchy
{
	ItemImageProvider::ItemImageProvider (const ICoreProxy_ptr& proxy)
	: Proxy_ { proxy }
	{
	}

	void ItemImageProvider::AddItem (Util::XDG::Item_ptr item)
	{
		PermID2Icon_ [item->GetPermanentID ()] = item->GetIcon (Proxy_);
	}

	QIcon ItemImageProvider::GetIcon (const QStringList& list)
	{
		auto id = list.at (0);
		if (PermID2Icon_.contains (id))
			return PermID2Icon_.value (id);

		id = QUrl::fromPercentEncoding (id.toUtf8 ());
		return PermID2Icon_.value (id);
	}
}
}
