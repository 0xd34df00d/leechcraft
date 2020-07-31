/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QDBusConnection>

namespace LC
{
namespace Liznoo
{
namespace Logind
{
	class LogindConnector : public QObject
	{
		Q_OBJECT

		QDBusConnection SB_ = QDBusConnection::connectToBus (QDBusConnection::SystemBus,
				"LeechCraft.Liznoo.Logind.LogindConnector");
		bool PowerEventsAvailable_ = false;
	public:
		LogindConnector (QObject* = nullptr);

		bool ArePowerEventsAvailable () const;
	private:
		void Inhibit ();
	private slots:
		void handlePreparing (bool);
	signals:
		void gonnaSleep (int);
		void wokeUp ();
	};

	using LogindConnector_ptr = std::shared_ptr<LogindConnector>;
}
}
}
