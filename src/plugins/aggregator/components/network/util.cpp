/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QNetworkRequest>
#include <interfaces/core/icoreproxy.h>
#include <util/sll/qtutil.h>

namespace LC::Aggregator
{
	void SetStandardHeaders (QNetworkRequest& req)
	{
		req.setHeader (QNetworkRequest::UserAgentHeader, "LeechCraft.Aggregator/"_qba + GetProxyHolder ()->GetVersion ().toLatin1 ());
	}
}
