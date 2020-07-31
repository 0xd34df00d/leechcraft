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
#include <QtPlugin>

namespace LC
{
namespace AdvancedNotifications
{
	class INotificationHandler;

	typedef std::shared_ptr<INotificationHandler> INotificationHandler_ptr;

	class INotificationBackendPlugin
	{
	public:
		virtual ~INotificationBackendPlugin () {}

		virtual QList<INotificationHandler_ptr> GetNotificationHandlers () const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::AdvancedNotifications::INotificationBackendPlugin,
		"org.LeechCraft.AdvancedNotifications.INotificationBackendPlugin/1.0")
