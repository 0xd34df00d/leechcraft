/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "coreproxyserverwrapper.h"

namespace LC
{
namespace DBus
{
	CoreProxyServerWrapper::CoreProxyServerWrapper (ICoreProxy *w, QObject *parent)
	: QDBusAbstractAdaptor { parent }
	, W_ { w }
	{
	}
}
}
