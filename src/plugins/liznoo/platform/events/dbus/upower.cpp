/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "upower.h"
#include <QtDebug>
#include <util/sll/qtutil.h>

namespace LC::Liznoo::Events
{
	const DBusPlatformBase::Config UPower::Config
	{
		.Service = "org.freedesktop.UPower"_qs,
		.Path = "/org/freedesktop/UPower"_qs,
		.PowerEventsSignals = { "\"Sleeping\""_qs, "\"Resuming\""_qs },
	};

	UPower::UPower (bool available, QObject *parent)
	: DBusPlatform { available, parent }
	, UPower_ { { .Service = Config.Service, .Path = Config.Path, .Interface = "org.freedesktop.UPower"_qs, .Conn = SB_ } }
	{
		if (!available)
		{
			qDebug () << "no Sleeping() or Resuming() signals, we are probably on systemd";
			return;
		}

		const auto sleepConnected = UPower_.Connect ("Sleeping", [this] { NotifyGonnaSleep (1000); });
		const auto resumeConnected = UPower_.Connect ("Resuming", this, &UPower::NotifyWokeUp);
		IsAvailable_ = sleepConnected && resumeConnected;
	}
}
