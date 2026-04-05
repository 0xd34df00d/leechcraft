/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/dbus/async.h>
#include <util/threads/coro/taskfwd.h>
#include "batteryplatform.h"

namespace LC::Liznoo::Battery
{
	class UPowerPlatform : public BatteryPlatform
	{
		Q_OBJECT

		QSet<QString> SubscribedDevices_;
		Util::DBus::EndpointWithSignals UPower_;
	public:
		explicit UPowerPlatform (QObject* = nullptr);
	private:
		Util::ContextTask<void> RequeryDevice (QString);
		Util::ContextTask<void> EnumerateDevices ();
	private slots:
		void handlePropertiesChanged (const QDBusMessage&);
	};
}
