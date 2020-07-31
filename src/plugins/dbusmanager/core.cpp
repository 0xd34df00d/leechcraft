/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "core.h"
#include <QtDebug>
#include <QDBusError>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QTimer>
#include <interfaces/iinfo.h>
#include <interfaces/structures.h>
#include <interfaces/iwebfilestorage.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#ifdef Q_OS_WIN32
#include <QProcess>
#endif
#include "generaladaptor.h"
#include "tasksadaptor.h"
#include "webfilestorageadaptor.h"

namespace LC
{
namespace DBusManager
{
	Core::Core ()
	{
		QTimer::singleShot (1500,
				this,
				SLOT (doDelayedInit ()));
#ifdef Q_OS_WIN32
		QProcess *daemon = new QProcess (this);
		daemon->start ("dbus/bin/dbus-daemon --session");
#endif
	}

	Core& Core::Instance ()
	{
		static Core core;
		return core;
	}

	void Core::Release ()
	{
		Proxy_.reset ();
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}

	void Core::doDelayedInit ()
	{
		General_.reset (new General);
		new GeneralAdaptor (General_.get ());

		Tasks_.reset (new Tasks);
		new TasksAdaptor (Tasks_.get ());

		QDBusConnection::sessionBus ().registerService ("org.LeechCraft.DBus");
		QDBusConnection::sessionBus ().registerObject ("/General", General_.get ());
		QDBusConnection::sessionBus ().registerObject ("/Tasks", Tasks_.get ());

		for (const auto root : Proxy_->GetPluginsManager ()->GetAllCastableRoots<IWebFileStorage*> ())
		{
			new WebFileStorageAdaptor (root);

			auto ii = qobject_cast<IInfo*> (root);
			const auto& name = "/WebFileStorage/" + ii->GetUniqueID ().replace ('.', '_');

			QDBusConnection::sessionBus ().registerObject (name, root);
		}
	}
}
}
