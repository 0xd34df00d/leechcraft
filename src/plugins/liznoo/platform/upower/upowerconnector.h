/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QSet>
#include "../../batteryinfo.h"
#include "../common/connectorbase.h"

class QDBusMessage;

namespace LC
{
namespace Liznoo
{
namespace UPower
{
	class UPowerConnector : public ConnectorBase
	{
		Q_OBJECT

		bool HasGlobalDeviceChanged_;
		QSet<QString> SubscribedDevices_;
	public:
		UPowerConnector (QObject* = nullptr);
	private:
		void ConnectChangedNotification ();
	private slots:
		void handleGonnaSleep ();
		void enumerateDevices ();
		void requeryDevice (const QString&);
		void handlePropertiesChanged (const QDBusMessage&);
	signals:
		void batteryInfoUpdated (Liznoo::BatteryInfo);
	};

	using UPowerConnector_ptr = std::shared_ptr<UPowerConnector>;
}
}
}
