/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "proxyfactory.h"
#include "proxiesstorage.h"

namespace LC
{
namespace XProxy
{
	ProxyFactory::ProxyFactory (ProxiesStorage *storage)
	: Storage_ { storage }
	{
	}

	QList<QNetworkProxy> ProxyFactory::queryProxy (const QNetworkProxyQuery& query)
	{
		QList<QNetworkProxy> proxies;
		if (query.queryType () == QNetworkProxyQuery::TcpSocket ||
				query.queryType () == QNetworkProxyQuery::UrlRequest)
		{
			const auto& matches = Storage_->FindMatching (query.peerHostName (),
					query.peerPort (), query.protocolTag ());
			std::copy (matches.begin (), matches.end (), std::back_inserter (proxies));
		}

		if (proxies.isEmpty ())
			proxies << QNetworkProxy (QNetworkProxy::NoProxy);

		return proxies;
	}
}
}
