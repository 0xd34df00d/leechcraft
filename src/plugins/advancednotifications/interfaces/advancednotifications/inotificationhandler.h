/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QList>
#include <QObject>
#include "types.h"

namespace LC
{
struct Entity;

namespace AdvancedNotifications
{
	class INotificationRule;

	class INotificationHandler
	{
	public:
		virtual ~INotificationHandler () = default;

		virtual NotificationMethod GetHandlerMethod () const = 0;

		/** @brief Handles an event or its cancellation.
		 *
		 * Called once per event with the rules that matched it and ask for
		 * this handler's method, and once per cancellation with no rules.
		 */
		virtual void Handle (const Entity&, const QList<const INotificationRule*>& rules) = 0;
	};

	using INotificationHandler_ptr = std::shared_ptr<INotificationHandler>;
}
}

Q_DECLARE_INTERFACE (LC::AdvancedNotifications::INotificationHandler,
		"org.LeechCraft.AdvancedNotifications.INotificationHandler/1.0")
