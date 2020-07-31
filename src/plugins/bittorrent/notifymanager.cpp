/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "notifymanager.h"
#include <QTimer>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include "core.h"

namespace LC
{
namespace BitTorrent
{
	NotifyManager::NotifyManager (QObject *parent)
	: QObject (parent)
	, IsReady_ (false)
	{
	}

	void NotifyManager::PluginsAvailable()
	{
		QTimer::singleShot (3000,
				this,
				SLOT (makeDelayedReady ()));
	}

	void NotifyManager::AddNotification (const Entity& e)
	{
		if (!IsReady_)
			Queue_ << e;
		else
			SendNotification (e);
	}

	void NotifyManager::SendNotification (const Entity& e)
	{
		Core::Instance ()->GetProxy ()->GetEntityManager ()->HandleEntity (e);
	}

	void NotifyManager::makeDelayedReady ()
	{
		IsReady_ = true;
		for (const auto& e : Queue_)
			SendNotification (e);
		Queue_.clear ();
	}
}
}
