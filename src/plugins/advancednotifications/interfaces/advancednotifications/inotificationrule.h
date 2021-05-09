/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

class QColor;

namespace LC::AdvancedNotifications
{
	class INotificationRule
	{
	public:
		virtual ~INotificationRule () = default;

		virtual bool IsNull () const = 0;

		virtual QColor GetColor () const = 0;

		bool operator== (const INotificationRule&) const = default;
	};
}
