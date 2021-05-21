/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "concretehandlerbase.h"
#include "notificationrule.h"

namespace LC::AdvancedNotifications
{
	void ConcreteHandlerBase::Handle (const Entity& e, const INotificationRule& rule)
	{
		Handle (e, dynamic_cast<const NotificationRule&> (rule));
	}
}
