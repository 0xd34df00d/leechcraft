/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "shortcutproxywrapper.h"

namespace LC
{
namespace Qrosp
{
	ShortcutProxyWrapper::ShortcutProxyWrapper (IShortcutProxy *proxy)
	: ShortcutProxy_ (proxy)
	{
	}

	QList<QKeySequence> ShortcutProxyWrapper::GetShortcuts (QObject *object, const QString& id)
	{
		return ShortcutProxy_->GetShortcuts (object, id);
	}
}
}
