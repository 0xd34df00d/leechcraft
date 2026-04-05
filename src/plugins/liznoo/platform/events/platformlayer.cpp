/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "platformlayer.h"
#include <util/xpc/util.h>
#include <interfaces/entityconstants.h>
#include <interfaces/core/ientitymanager.h>

namespace LC::Liznoo::Events
{
	bool PlatformLayer::IsAvailable () const
	{
		return IsAvailable_;
	}

	void PlatformLayer::NotifyGonnaSleep (int timeout)
	{
		qDebug () << "detected sleep in" << timeout;
		auto e = Util::MakeEntity (PowerState::Sleeping,
				{},
				TaskParameter::Internal,
				Mimes::PowerStateChanged);
		e.Additional_ [EF::PowerState::TimeLeft] = timeout;
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	void PlatformLayer::NotifyWokeUp ()
	{
		qDebug () << "detected wake up";
		const auto& e = Util::MakeEntity (PowerState::WokeUp,
				{},
				TaskParameter::Internal,
				Mimes::PowerStateChanged);
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}
}
