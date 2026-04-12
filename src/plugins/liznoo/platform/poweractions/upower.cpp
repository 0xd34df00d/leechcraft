/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "upower.h"
#include <util/threads/coro.h>
#include <util/threads/coro/dbus.h>
#include "platform/dbus/endpoints.h"

namespace LC::Liznoo::PowerActions
{
	namespace
	{
		QString State2Method (Platform::State state)
		{
			switch (state)
			{
			case Platform::State::Suspend:
				return "Suspend"_qs;
			case Platform::State::Hibernate:
				return "Hibernate"_qs;
			}
			std::unreachable ();
		}
	}

	Util::ContextTask<bool> UPower::IsAvailable () const
	{
		const auto& upower = DBus::GetUPowerEndpoint ();
		const auto eitherCanSuspend = co_await upower.GetProperty<bool> ("CanSuspend"_qs);
		const auto eitherCanHibernate = co_await upower.GetProperty<bool> ("CanHibernate"_qs);
		co_return eitherCanSuspend.IsRight () && eitherCanHibernate.IsRight ();
	}

	Util::ContextTask<Platform::Result> UPower::CanChangeState (State state)
	{
		const auto& upower = DBus::GetUPowerEndpoint ();
		const auto eitherAllowed = co_await upower.GetProperty<bool> ("Can"_ql + State2Method (state));
		if (co_await Util::WithHandler (eitherAllowed, [] (const QDBusError& err) { return err.message (); }))
			co_return Success {};
		co_return Fail { tr ("Cannot change state: either not supported or insufficient permissions.") };
	}

	Util::ContextTask<Platform::Result> UPower::ChangeState (State state)
	{
		const auto& upower = DBus::GetUPowerEndpoint ();

		const auto res = co_await upower.Call<> (State2Method (state));
		co_return co_await Util::WithHandler (res,
				[] (const QDBusError& err) { return tr ("Failed to change state: %1.").arg (err.message ()); });
	}
}
