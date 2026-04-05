/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dbusplatformbase.h"
#include <algorithm>
#include <QtDebug>
#include <util/sll/qtutil.h>

namespace LC::Liznoo::Events
{
	DBusPlatformBase::DBusPlatformBase (bool available, QObject *parent)
	: Platform { parent }
	{
		IsAvailable_ = available;
	}

	Util::ContextTask<bool> DBusPlatformBase::CheckSignals (Config config)
	{
		const auto msg = QDBusMessage::createMethodCall (config.Service,
				config.Path,
				"org.freedesktop.DBus.Introspectable"_qs,
				"Introspect"_qs);
		const auto& eitherIntrospect = co_await Util::Typed<QString> (QDBusConnection::systemBus ().asyncCall (msg));
		if (const auto err = eitherIntrospect.MaybeLeft ())
		{
			qWarning () << "introspect failed at" << config.Path << "with" << *err;
			co_return false;
		}
		const auto& introspect = eitherIntrospect.GetRight ();
		co_return std::ranges::all_of (config.PowerEventsSignals,
				[&introspect] (const QString& signal) { return introspect.contains (signal); });
	}
}
