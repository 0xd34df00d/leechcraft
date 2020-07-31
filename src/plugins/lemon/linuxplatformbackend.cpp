/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "linuxplatformbackend.h"
#include <QStringList>
#include <QtDebug>
#include <netlink/route/addr.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/link.h>
#include <netlink/route/route.h>

namespace LC
{
namespace Lemon
{
	LinuxPlatformBackend::LinuxPlatformBackend (QObject *parent)
	: PlatformBackend (parent)
	, Rtsock_ (nl_socket_alloc ())
	{
		if (nl_connect (Rtsock_, NETLINK_ROUTE) >= 0)
			rtnl_link_alloc_cache (Rtsock_, AF_UNSPEC, &LinkCache_);
		else
			qWarning () << Q_FUNC_INFO
					<< "unable to establish netlink conn";
	}

	LinuxPlatformBackend::~LinuxPlatformBackend ()
	{
		nl_cache_free (LinkCache_);
		nl_close (Rtsock_);
		nl_socket_free (Rtsock_);
	}

	auto LinuxPlatformBackend::GetCurrentNumBytes (const QString& name) const -> CurrentTrafficState
	{
		return DevInfos_ [name].Traffic_;
	}

	void LinuxPlatformBackend::update (const QStringList& devices)
	{
		if (!LinkCache_)
			return;

		nl_cache_refill (Rtsock_, LinkCache_);

		for (const auto& devName : devices)
		{
			auto link = rtnl_link_get_by_name (LinkCache_, devName.toLocal8Bit ().constData ());
			if (!link)
			{
				qWarning () << Q_FUNC_INFO
						<< "no link for device"
						<< devName;
				continue;
			}

			auto& info = DevInfos_ [devName];

			info.Traffic_.Down_ = rtnl_link_get_stat (link, RTNL_LINK_RX_BYTES);
			info.Traffic_.Up_ = rtnl_link_get_stat (link, RTNL_LINK_TX_BYTES);

			rtnl_link_put (link);
		}
	}
}
}
