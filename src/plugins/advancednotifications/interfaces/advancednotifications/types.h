/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QFlags>

namespace LC::AdvancedNotifications
{
	enum NotificationMethod
	{
		NMNone = 0x00,
		NMVisual = 0x01,
		NMTray = 0x02,
		NMAudio = 0x04,
		NMCommand = 0x08,
		NMUrgentHint = 0x10,
		NMSystemDependent = 0x20
	};

	Q_DECLARE_FLAGS (NotificationMethods, NotificationMethod);
}

Q_DECLARE_OPERATORS_FOR_FLAGS (LC::AdvancedNotifications::NotificationMethods)
