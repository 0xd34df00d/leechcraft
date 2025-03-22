/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "laughty.h"
#include <QIcon>
#include <QDBusConnection>
#include "serverobject.h"
#include "serveradaptor.h"

namespace LC
{
namespace Laughty
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		if (!QDBusConnection::sessionBus ().registerService ("org.freedesktop.Notifications"))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to register Notifications service."
					<< "Is another notification daemon active?";
			return;
		}

		auto server = new ServerObject (proxy);
		new ServerAdaptor (server, proxy);
		QDBusConnection::sessionBus ().registerObject ("/org/freedesktop/Notifications", server);
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Laughty";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Laughty";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Desktop Notifications server.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_laughty, LC::Laughty::Plugin);
