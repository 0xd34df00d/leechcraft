/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dolle.h"
#include <QIcon>
#include "notificationhandler.h"
#include "dockutil.h"

namespace LC
{
namespace AdvancedNotifications
{
namespace Dolle
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		DU::InstallBadgeView ();
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.AdvancedNotifications.Dolle";
	}

	QString Plugin::GetName () const
	{
		return "AdvancedNotifications Dolle";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("OS X notifications backend.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.AdvancedNotifications.NotificationsBackend" };
	}

	QList<INotificationHandler_ptr> Plugin::GetNotificationHandlers () const
	{
		return { std::make_shared<NotificationHandler> () };
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_advancednotifications_dolle, LC::AdvancedNotifications::Dolle::Plugin);
