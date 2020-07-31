/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "freedesktop.h"
#include <QTimer>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCall>

namespace LC
{
namespace Liznoo
{
namespace Screen
{
	Freedesktop::Freedesktop (QObject *parent)
	: ScreenPlatform (parent)
	, ActivityTimer_ (new QTimer (this))
	{
		connect (ActivityTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (handleTimeout ()));
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

	void Freedesktop::handleTimeout ()
	{
		QDBusInterface iface ("org.freedesktop.ScreenSaver", "/ScreenSaver");
		iface.call ("SimulateUserActivity");
	}
}
}
}
