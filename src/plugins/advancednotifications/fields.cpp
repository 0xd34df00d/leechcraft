/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fields.h"
#include <QString>
#include <util/sll/qtutil.h>

namespace LC::AdvancedNotifications::Fields
{
	const QString SenderID = QStringLiteral ("org.LC.AdvNotifications.SenderID");
	const QString EventID = QStringLiteral ("org.LC.AdvNotifications.EventID");
	const QString EventCategory = QStringLiteral ("org.LC.AdvNotifications.CancelEvent");
	const QString EventType = QStringLiteral ("org.LC.AdvNotifications.EventType");

	namespace Values
	{
		const QLatin1String CancelEvent = "org.LC.AdvNotifications.Cancel"_ql;
	}
}
