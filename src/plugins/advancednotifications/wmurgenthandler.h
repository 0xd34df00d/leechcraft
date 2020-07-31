/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "concretehandlerbase.h"

namespace LC
{
namespace AdvancedNotifications
{
	class WMUrgentHandler : public ConcreteHandlerBase
	{
		Q_OBJECT
	public:
		WMUrgentHandler ();

		NotificationMethod GetHandlerMethod () const;
		void Handle (const Entity&, const NotificationRule&);
	};
}
}
