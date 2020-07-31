/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "notificationhandler.h"
#include <numeric>
#include <QtDebug>
#include <interfaces/structures.h>
#include <interfaces/advancednotifications/inotificationrule.h>
#include <interfaces/advancednotifications/types.h>
#include "dockutil.h"

namespace LC
{
namespace AdvancedNotifications
{
namespace Dolle
{
	NotificationMethod NotificationHandler::GetHandlerMethod () const
	{
		return NMTray;
	}

	void NotificationHandler::Handle (const Entity& e, const INotificationRule& rule)
	{
		const QString& cat = e.Additional_ ["org.LC.AdvNotifications.EventCategory"].toString ();
		const QString& type = e.Additional_ ["org.LC.AdvNotifications.EventType"].toString ();
		const QString& eventId = e.Additional_ ["org.LC.AdvNotifications.EventID"].toString ();

		auto& data = Counts_ [type];

		if (cat != "org.LC.AdvNotifications.Cancel")
		{
			if (const int delta = e.Additional_.value ("org.LC.AdvNotifications.DeltaCount", 0).toInt ())
				data.Counts_ [eventId] += delta;
			else
				data.Counts_ [eventId] = e.Additional_.value ("org.LC.AdvNotifications.Count", 1).toInt ();

			data.Color_ = rule.GetColor ();
			data.Total_ = std::accumulate (data.Counts_.constBegin (), data.Counts_.constEnd (), 0);
		}
		else
		{
			QMutableMapIterator<QString, NotificationData> it { Counts_ };
			bool removed = false;
			while (it.hasNext () && !removed)
			{
				NotificationData& nd = it.next ().value ();
				if (nd.Counts_.remove (eventId))
				{
					nd.Total_ = std::accumulate (data.Counts_.constBegin (), data.Counts_.constEnd (), 0);
					removed = true;
				}
			}
			if (!removed)
				return;
		}

		DU::SetDockBadges (Counts_.values ());
	}
}
}
}
