/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "logind.h"
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

	Util::ContextTask<bool> Logind::IsAvailable () const
	{
		const auto& logind = DBus::GetLogindEndpoint ();
		const auto eitherCanSuspend = co_await logind.Call<QString> ("CanSuspend"_qs);
		co_return eitherCanSuspend.IsRight ();
	}

	Util::ContextTask<Platform::Result> Logind::CanChangeState (State state)
	{
		const auto& logind = DBus::GetLogindEndpoint ();
		const auto eitherResponse = co_await logind.Call<QString> ("Can"_ql + State2Method (state));
		const auto response = co_await Util::WithHandler (eitherResponse,
				[] (const QDBusError& err) { return err.message (); });
		if (response == "yes"_qs || response == "challenge"_qs)
			co_return Success {};
		if (response == "no"_qs)
			co_return Fail { tr ("Target state is not allowed.") };
		if (response == "na"_qs)
			co_return Fail { tr ("Target state is not supported on this system.") };

		qWarning () << static_cast<int> (state) << response;
		co_return Fail { tr ("Unknown response: %1.").arg (response) };
	}

	Util::ContextTask<Platform::Result> Logind::ChangeState (State state)
	{
		const auto& logind = DBus::GetLogindEndpoint ();

		const auto interactive = true;
		const auto res = co_await logind.Call<> (State2Method (state), interactive);
		co_return co_await Util::WithHandler (res,
				[] (const QDBusError& err) { return tr ("Failed to change state: %1.").arg (err.message ()); });
	}
}
