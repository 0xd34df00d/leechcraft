/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "themeimageprovider.h"
#include <QIcon>
#include "interfaces/core/iiconthememanager.h"

namespace LC::Util
{
	ThemeImageProvider::ThemeImageProvider (ICoreProxy_ptr proxy)
	: Proxy_ (proxy)
	{
	}

	QIcon ThemeImageProvider::GetIcon (const QStringList& list)
	{
		return Proxy_->GetIconThemeManager ()->GetIcon (list.value (0));
	}
}
