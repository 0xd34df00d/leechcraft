/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <util/sll/qtutil.h>
#include <interfaces/core/icoreproxy.h>

namespace LC
{
namespace MusicZombie
{
	QNetworkRequest SetupRequest (QNetworkRequest req)
	{
		const auto userAgent = "LeechCraft MusicZombie/%1 ( 0xd34df00d@gmail.com )"_qs
				.arg (GetProxyHolder ()->GetVersion ());
		req.setHeader (QNetworkRequest::UserAgentHeader, userAgent);
		return req;
	}
}
}
