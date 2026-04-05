/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "consolekit.h"
#include <util/sll/qtutil.h>

namespace LC::Liznoo::Events
{
	const DBusPlatformBase::Config ConsoleKit::Config
	{
		.Service = "org.freedesktop.ConsoleKit"_qs,
		.Path = "/org/freedesktop/ConsoleKit/Manager"_qs,
		.PowerEventsSignals = { "\"PrepareForSleep\""_qs },
	};

	ConsoleKit::ConsoleKit (bool powerSignalsAvailable, QObject *parent)
	: DBusPlatform { powerSignalsAvailable, parent }
	, Endpoint_ { { .Service = Config.Service, .Path = Config.Path, .Interface = "org.freedesktop.ConsoleKit.Manager"_qs, .Conn = SB_ } }
	{
		if (powerSignalsAvailable)
			Endpoint_.Connect ("PrepareForSleep",
					[this] (bool active)
					{
						if (active)
							NotifyGonnaSleep (1000);
						else
							NotifyWokeUp ();
					});
	}
}
