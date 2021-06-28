/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/structures.h>
#include "concretehandlerbase.h"
#include "notificationrule.h"

namespace LC::AdvancedNotifications
{
	class VisualHandler : public ConcreteHandlerBase
	{
		QSet<QString> ActiveEvents_;
	public:
		NotificationMethod GetHandlerMethod () const override;
		void Handle (const Entity&, const NotificationRule&) override;
	};
}
