/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "linuxplatformbackend.h"
#include <QStringList>
#include <QtDebug>
#include <netlink/route/addr.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/link.h>
#include <netlink/route/route.h>

namespace LeechCraft
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
		}
	}
}
}
