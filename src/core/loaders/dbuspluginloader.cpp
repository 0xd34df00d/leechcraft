/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dbuspluginloader.h"
#include <QProcess>
#include <QDir>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include <QLocalServer>
#include <interfaces/iinfo.h>
#include "infoproxy.h"
#include "dbus/marshalling.h"

namespace LC
{
namespace Loaders
{
	DBusPluginLoader::DBusPluginLoader (const QString& filename)
	: Filename_ (filename)
	, IsLoaded_ (false)
	, Proc_ (new QProcess)
	{
		DBus::RegisterTypes ();

		auto sb = QDBusConnection::sessionBus ();
		const QString serviceName { "org.LeechCraft.MainInstance" };

		if (!sb.interface ()->isServiceRegistered (serviceName))
			qDebug () << "registering primary service..." << sb.registerService (serviceName);

		connect (Proc_.get (),
				SIGNAL (finished (int, QProcess::ExitStatus)),
				this,
				SLOT (handleProcFinished ()));
	}

	quint64 DBusPluginLoader::GetAPILevel ()
	{
		return GetLibAPILevel (Filename_);
	}

	bool DBusPluginLoader::Load ()
	{
		if (IsLoaded ())
			return true;

		Proc_->start ("lc_plugin_wrapper", QStringList {});

		QLocalServer srv;
		srv.listen (QString ("lc_waiter_%1").arg (Proc_->processId ()));

		if (!Proc_->waitForStarted ())
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot start proc";
			return false;
		}

		if (!srv.waitForNewConnection (1000))
		{
			qWarning () << Q_FUNC_INFO
					<< "time out waiting for connection"
					<< Filename_;
			return false;
		}

		const auto& serviceName = QString ("org.LeechCraft.Wrapper_%1").arg (Proc_->processId ());
		CtrlIface_.reset (new QDBusInterface (serviceName,
					"/org/LeechCraft/Control",
					"org.LeechCraft.Control"));

		QDBusReply<bool> reply = CtrlIface_->call ("Load", Filename_);
		IsLoaded_ = reply.value ();
		qDebug () << Q_FUNC_INFO
				<< GetFileName ()
				<< "is loaded?"
				<< IsLoaded_;
		if (!IsLoaded_)
			return false;

		Wrapper_.reset (new InfoProxy (serviceName));

		CtrlIface_->call ("SetLcIconsPaths", QDir::searchPaths ("lcicons"));

		return true;
	}

	bool DBusPluginLoader::Unload ()
	{
		if (!IsLoaded ())
			return true;

		QDBusReply<bool> reply = CtrlIface_->call ("Unload", Filename_);
		if (reply.value ())
		{
			CtrlIface_.reset ();
			IsLoaded_ = false;
		}

		return !IsLoaded_;
	}

	QObject* DBusPluginLoader::Instance ()
	{
		if (!IsLoaded () && !Load ())
		{
			qWarning () << Q_FUNC_INFO
					<< "null instance";
			return 0;
		}

		return Wrapper_.get ();
	}

	bool DBusPluginLoader::IsLoaded () const
	{
		return IsLoaded_;
	}

	QString DBusPluginLoader::GetFileName () const
	{
		return Filename_;
	}

	QString DBusPluginLoader::GetErrorString () const
	{
		return {};
	}

	QVariantMap DBusPluginLoader::GetManifest () const
	{
		// TODO
		return {};
	}

	void DBusPluginLoader::handleProcFinished ()
	{
		qDebug () << Q_FUNC_INFO << Proc_->exitCode () << Proc_->exitStatus ();
		qDebug () << Proc_->readAllStandardOutput ();
		qDebug () << Proc_->readAllStandardError ();
	}
}
}
