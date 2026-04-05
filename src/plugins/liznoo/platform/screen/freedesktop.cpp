/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "freedesktop.h"
#include <QTimer>
#include <util/dbus/async.h>
#include <util/threads/coro.h>
#include <util/threads/coro/dbus.h>

namespace LC::Liznoo::Screen
{
	namespace
	{
		void SimulateUserActivity ()
		{
			const Util::DBus::Endpoint screensaver
			{
				.Service = "org.freedesktop.ScreenSaver"_qs,
				.Path = "/ScreenSaver"_qs,
				.Interface = "org.freedesktop.ScreenSaver"_qs,
			};
			auto _ = screensaver.Call<> ("SimulateUserActivity"_qs);
		}
	}

	Freedesktop::Freedesktop (QObject *parent)
	: Platform (parent)
	, ActivityTimer_ (new QTimer (this))
	{
		ActivityTimer_->callOnTimeout (&SimulateUserActivity);
		ActivityTimer_->setInterval (30000);
	}

	void Freedesktop::ProhibitScreensaver (bool prohibit, const QString& id)
	{
		if (prohibit)
		{
			if (ActiveProhibitions_.isEmpty ())
				ActivityTimer_->start ();

			ActiveProhibitions_ << id;
		}
		else
		{
			ActiveProhibitions_.remove (id);

			if (ActiveProhibitions_.isEmpty ())
				ActivityTimer_->stop ();
		}
	}
}
