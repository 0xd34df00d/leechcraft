/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include "platformlayer.h"
#include "../common/dbusthread.h"

namespace LC
{
namespace Liznoo
{
namespace Events
{
	template<typename ConnT>
	class PlatformUPowerLike : public PlatformLayer
	{
		using DBusThread_ptr = std::shared_ptr<DBusThread<ConnT>>;

		const DBusThread_ptr Thread_;
	public:
		PlatformUPowerLike (const DBusThread_ptr& thread,
				const ICoreProxy_ptr& proxy, QObject *parent = nullptr)
		: PlatformLayer { proxy, parent }
		, Thread_ { thread }
		{
			Thread_->ScheduleImpl ([this] (ConnT *conn)
					{
						connect (conn,
								SIGNAL (gonnaSleep (int)),
								this,
								SLOT (emitGonnaSleep (int)));
						connect (conn,
								SIGNAL (wokeUp ()),
								this,
								SLOT (emitWokeUp ()));

						QMetaObject::invokeMethod (this,
								"setAvailable",
								 Qt::QueuedConnection,
								 Q_ARG (bool, conn->ArePowerEventsAvailable ()));
					});
		}
	};

	template<typename ConnT>
	using PlatformUPowerLike_ptr = std::shared_ptr<PlatformUPowerLike<ConnT>>;

	template<typename ConnT>
	PlatformUPowerLike_ptr<ConnT> MakeUPowerLike (const DBusThread_ptr<ConnT>& thread, const ICoreProxy_ptr& proxy)
	{
		return std::make_shared<PlatformUPowerLike<ConnT>> (thread, proxy);
	}
}
}
}
