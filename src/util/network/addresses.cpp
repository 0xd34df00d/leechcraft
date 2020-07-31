/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "addresses.h"
#include <QHostAddress>
#include <QNetworkInterface>

namespace LC
{
namespace Util
{
	AddrList_t GetLocalAddresses (int defaultPort)
	{
		AddrList_t defaultAddrs;
		const auto locals =
		{
			QHostAddress::parseSubnet ("10.0.0.0/8"),
			QHostAddress::parseSubnet ("172.16.0.0/12"),
			QHostAddress::parseSubnet ("192.168.0.0/16")
		};
		for (const auto& addr : GetAllAddresses ())
			if (std::any_of (std::begin (locals), std::end (locals),
					[&addr] (const auto& subnet) { return addr.isInSubnet (subnet); }))
				defaultAddrs.push_back ({ addr.toString (), QString::number (defaultPort) });
		return defaultAddrs;
	}

	QList<QHostAddress> GetAllAddresses ()
	{
		QList<QHostAddress> result;
		for (const auto& addr : QNetworkInterface::allAddresses ())
			if (addr.scopeId ().isEmpty ())
				result << addr;

		if (!result.contains (QHostAddress::Any))
			result << QHostAddress::Any;

		return result;
	}

}
}
