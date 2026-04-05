/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "endpoints.h"
#include <util/sll/qtutil.h>

namespace LC::Liznoo::DBus
{
	Util::DBus::Endpoint GetUPowerEndpoint ()
	{
		return
		{
			.Service = "org.freedesktop.UPower"_qs,
			.Path = "/org/freedesktop/UPower"_qs,
			.Interface = "org.freedesktop.UPower"_qs,
			.Conn = QDBusConnection::systemBus (),
		};
	}
}
