/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include "interfaces/advancednotifications/inotificationhandler.h"

namespace LC
{
struct Entity;

namespace AdvancedNotifications
{
	class GeneralHandler;
	class NotificationRule;

	class ConcreteHandlerBase : public QObject
							  , public INotificationHandler
	{
	public:
		virtual void Handle (const Entity&, const NotificationRule&) = 0;
		void Handle (const Entity&, const INotificationRule&) override;
	};

	using ConcreteHandlerBase_ptr = std::shared_ptr<ConcreteHandlerBase>;
}
}
