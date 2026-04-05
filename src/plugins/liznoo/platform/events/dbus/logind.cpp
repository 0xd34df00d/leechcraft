/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "logind.h"
#include <QDBusUnixFileDescriptor>
#include <QtDebug>
#include <unistd.h>
#include <util/sll/qtutil.h>

namespace LC::Liznoo::Events
{
	const DBusPlatformBase::Config Logind::Config
	{
		.Service = "org.freedesktop.login1"_qs,
		.Path = "/org/freedesktop/login1"_qs,
		.PowerEventsSignals = { "\"PreparingForSleep\""_qs },
	};

	Logind::Logind (bool available, QObject *parent)
	: DBusPlatform { available, parent }
	, Logind_ { { .Service = "org.freedesktop.login1", .Path = "/org/freedesktop/login1", .Interface = "org.freedesktop.login1.Manager", .Conn = SB_ } }
	{
		if (available)
			Logind_.Connect ("PreparingForSleep"_qs,
					[this] (bool goingDown)
					{
						if (goingDown)
						{
							Inhibit ();
							NotifyGonnaSleep (5000);
						}
						else
							NotifyWokeUp ();
					});
	}

	Util::Task<void> Logind::Inhibit ()
	{
		const auto eitherResponse = co_await Logind_.Call<QDBusUnixFileDescriptor> ("Inhibit"_qs,
					"shutdown:sleep"_qs,
					"LeechCraft"_qs,
					tr ("Preparing LeechCraft for going to sleep..."),
					"delay"_qs);
		const auto fd = co_await Util::WithHandler (eitherResponse,
			[] (const auto& err) { qWarning () << "failed to inhibit sleep" << err; });
		qDebug () << fd.fileDescriptor ();
		close (fd.fileDescriptor ());
	}
}
