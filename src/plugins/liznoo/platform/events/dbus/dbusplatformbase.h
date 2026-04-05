/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/dbus/async.h>
#include <util/threads/coro.h>
#include <util/threads/coro/dbus.h>
#include "../platformlayer.h"

namespace LC::Liznoo::Events
{
	class DBusPlatformBase : public PlatformLayer
	{
	protected:
		struct Config
		{
			QString Service;
			QString Path;
			QStringList PowerEventsSignals;
		};

		QDBusConnection SB_ = QDBusConnection::systemBus ();

		explicit DBusPlatformBase (bool available, QObject *parent = nullptr);

		static Util::ContextTask<bool> CheckSignals (Config config);
	};

	template<typename Self>
	class DBusPlatform : public DBusPlatformBase
	{
	protected:
		using DBusPlatformBase::DBusPlatformBase;
	public:
		static Util::ContextTask<std::shared_ptr<Self>> Create ()
		{
			const auto& service = Self::Config.Service;
			if (const auto startResult = co_await Util::Typed<> (Util::DBus::StartService (QDBusConnection::systemBus (), service));
				const auto err = startResult.MaybeLeft ())
			{
				if (err->type () == QDBusError::ServiceUnknown)
					qDebug () << "service" << service << "is not available, skipping";
				else
					qWarning () << "failed to start" << service << ":" << *err;
				co_return {};
			}

			co_return std::make_shared<Self> (co_await CheckSignals (Self::Config));
		}
	};
}
