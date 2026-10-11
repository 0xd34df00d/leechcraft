/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QNetworkRequest>
#include <QUrl>
#include <interfaces/core/icoreproxy.h>
#include <util/sll/qtutil.h>

namespace LC::Aggregator
{
	void SetStandardHeaders (QNetworkRequest& req)
	{
		req.setHeader (QNetworkRequest::UserAgentHeader, "LeechCraft.Aggregator/"_qba + GetProxyHolder ()->GetVersion ().toLatin1 ());
	}

	QUrl FixupFeedScheme (QUrl url)
	{
		const auto& scheme = url.scheme ();
		constexpr auto feedMarker = "feed"_ql;
		constexpr auto itpcMarker = "itpc"_ql;
		if (scheme == feedMarker && url.host ().isEmpty ())
		{
			// feed:http://… kind of URLs
			return QUrl { url.toString ().sliced (feedMarker.size () + "://"_ql.size ()) };
		}

		if (scheme == feedMarker || scheme == itpcMarker)
			url.setScheme ("http"_qs);
		return url;
	}
}
