/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMap>
#include <interfaces/advancednotifications/inotificationhandler.h>
#include "notificationdata.h"

namespace LC
{
namespace AdvancedNotifications
{
namespace Dolle
{
	class NotificationHandler : public QObject
							  , public INotificationHandler
	{
		Q_OBJECT
		Q_INTERFACES (LC::AdvancedNotifications::INotificationHandler)

		QMap<QString, NotificationData> Counts_;
	public:
		NotificationMethod GetHandlerMethod () const override;
		void Handle (const Entity&, const INotificationRule&) override;
	};
}
}
}
