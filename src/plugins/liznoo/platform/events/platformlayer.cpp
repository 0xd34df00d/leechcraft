/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "platformlayer.h"
#include <QFuture>
#include <util/xpc/util.h>
#include <interfaces/core/ientitymanager.h>

namespace LC
{
namespace Liznoo
{
namespace Events
{
	PlatformLayer::PlatformLayer (const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject { parent }
	, Proxy_ { proxy }
	{
		IsAvailable_.reportStarted ();
	}

	QFuture<bool> PlatformLayer::IsAvailable ()
	{
		return IsAvailable_.future ();
	}

	void PlatformLayer::setAvailable (bool avail)
	{
		IsAvailable_.reportFinished (&avail);
	}

	void PlatformLayer::emitGonnaSleep (int timeout)
	{
		qDebug () << Q_FUNC_INFO << "detected sleep in" << timeout;
		auto e = Util::MakeEntity ("Sleeping",
				{},
				TaskParameter::Internal,
				"x-leechcraft/power-state-changed");
		e.Additional_ ["TimeLeft"] = timeout;
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}

	void PlatformLayer::emitWokeUp ()
	{
		qDebug () << Q_FUNC_INFO << "detected wake up";
		const auto& e = Util::MakeEntity ("WokeUp",
				{},
				TaskParameter::Internal,
				"x-leechcraft/power-state-changed");
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}
}
}
}
