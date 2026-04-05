/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "upower.h"
#include <QtDBus>
#include <util/threads/coro.h>
#include <util/threads/coro/dbus.h>

namespace LC::Liznoo::PowerActions
{
	namespace
	{
		QByteArray State2Method (Platform::State state)
		{
			switch (state)
			{
			case Platform::State::Suspend:
				return "Suspend";
			case Platform::State::Hibernate:
				return "Hibernate";
			}
			std::unreachable ();
		}
	}

	Util::ContextTask<bool> UPower::IsAvailable ()
	{
		const QDBusInterface face
		{
			"org.freedesktop.UPower",
			"/org/freedesktop/UPower",
			"org.freedesktop.UPower",
			QDBusConnection::systemBus ()
		};
		co_return face.isValid () &&
				face.property ("CanSuspend").isValid () &&
				face.property ("CanHibernate").isValid ();
	}

	Util::ContextTask<Platform::Result> UPower::CanChangeState (State state)
	{
		const QDBusInterface face
		{
			"org.freedesktop.UPower",
			"/org/freedesktop/UPower",
			"org.freedesktop.UPower",
			QDBusConnection::systemBus ()
		};
		if (!face.isValid ())
			co_return Fail { tr ("Cannot connect to UPower daemon.") };

		if (face.property ("Can" + State2Method (state)).toBool ())
			co_return Success {};
		co_return Fail {};
	}

	Util::ContextTask<Platform::Result> UPower::ChangeState (State state)
	{
		QDBusInterface face
		{
			"org.freedesktop.UPower",
			"/org/freedesktop/UPower",
			"org.freedesktop.UPower",
			QDBusConnection::systemBus ()
		};

		const auto res = co_await Util::Typed<> (face.asyncCall (State2Method (state)));
		co_return co_await Util::WithHandler (res,
				[] (const QDBusError& err) { return tr ("Failed to change state: %1.").arg (err.message ()); });
	}
}
