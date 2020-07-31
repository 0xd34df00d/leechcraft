/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "notificationsext.h"
#include <util/xpc/util.h>
#include <interfaces/core/ientitymanager.h>

namespace LC
{
namespace WKPlugins
{
	NotificationsExt::NotificationsExt (const ICoreProxy_ptr& proxy)
	: Proxy_ { proxy }
	{
	}

	void NotificationsExt::showNotification (const QWebNotificationData *data)
	{
		const auto& entity = Util::MakeNotification (data->title (),
				data->message (), Priority::Info);
		Proxy_->GetEntityManager ()->HandleEntity (entity);
	}
}
}
