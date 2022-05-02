/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "linuxplatformbackend.h"
#include <stdexcept>
#include <QStringList>
#include <QtDebug>
#include <netlink/errno.h>
#include <netlink/route/addr.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/link.h>
#include <netlink/route/route.h>

namespace LC::Lemon
{
	LinuxPlatformBackend::SockConn::SockConn (nl_sock& sock)
	: Sock_ { sock }
	{
		if (const auto res = nl_connect (&Sock_, NETLINK_ROUTE);
			res < 0)
		{
			const auto errStr = nl_geterror (res);
			qCritical () << Q_FUNC_INFO
					<< "unable to connect to netlink:"
					<< errStr;
			throw std::runtime_error { "unable to connect to netlink" };
		}
	}

	LinuxPlatformBackend::SockConn::~SockConn ()
	{
		nl_close (&Sock_);
	}

	namespace
	{
		auto MakeCache (nl_sock& sock)
		{
			nl_cache *cache = nullptr;
			rtnl_link_alloc_cache (&sock, AF_UNSPEC, &cache);
			return cache;
		}
	}

	LinuxPlatformBackend::LinuxPlatformBackend (QObject *parent)
	: PlatformBackend { parent }
	, Rtsock_ { nl_socket_alloc (), &nl_socket_free }
	, Conn_ { *Rtsock_ }
	, LinkCache_ { MakeCache (*Rtsock_), &nl_cache_free }
	{
	}

	LinuxPlatformBackend::~LinuxPlatformBackend () = default;

	auto LinuxPlatformBackend::GetCurrentNumBytes (const QString& name) const -> CurrentTrafficState
	{
		return DevInfos_ [name].Traffic_;
	}

	void LinuxPlatformBackend::Update (const QStringList& devices)
	{
		if (!LinkCache_)
			return;

		nl_cache_refill (&*Rtsock_, &*LinkCache_);

		for (const auto& devName : devices)
		{
			auto link = rtnl_link_get_by_name (&*LinkCache_, devName.toLocal8Bit ().constData ());
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
