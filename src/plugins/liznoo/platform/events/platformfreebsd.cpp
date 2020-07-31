/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 * Copyright (C) 2012       Maxim Ignatenko
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "platformfreebsd.h"
#include <QTimer>

namespace LC
{
namespace Liznoo
{
namespace Events
{
	PlatformFreeBSD::PlatformFreeBSD (const ICoreProxy_ptr& proxy, QObject *parent)
	: PlatformLayer (proxy, parent)
	{
		setAvailable (false);
	}
}
}
}
