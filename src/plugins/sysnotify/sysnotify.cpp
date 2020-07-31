/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sysnotify.h"
#include <QIcon>
#include <util/util.h>
#include <interfaces/entitytesthandleresult.h>
#include "notificationmanager.h"

namespace LC
{
namespace Sysnotify
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("sysnotify");

		Manager_ = std::make_shared<NotificationManager> ();
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Sysnotify";
	}

	void Plugin::Release ()
	{
		Manager_.reset ();
	}

	QString Plugin::GetName () const
	{
		return "Sysnotify";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Notifies about LeechCraft events via Desktop Notifications.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		return EntityTestHandleResult { Manager_.get () && Manager_->CouldNotify (e) ?
					EntityTestHandleResult::PHigh :
					EntityTestHandleResult::PNone };
	}

	void Plugin::Handle (Entity e)
	{
		Manager_->HandleNotification (e);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_sysnotify, LC::Sysnotify::Plugin);
