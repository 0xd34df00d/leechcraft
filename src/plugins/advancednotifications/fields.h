/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

class QString;
class QLatin1String;

namespace LC::AdvancedNotifications::Fields
{
	extern const QString SenderID;
	extern const QString EventID;
	extern const QString EventCategory;
	extern const QString EventType;

	namespace Values
	{
		extern const QLatin1String CancelEvent;
	}
}
