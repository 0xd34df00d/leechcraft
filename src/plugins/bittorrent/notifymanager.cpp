/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "notifymanager.h"
#include <QTimer>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include "core.h"

namespace LeechCraft
{
namespace Plugins
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
}
