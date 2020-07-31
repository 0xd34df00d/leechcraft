/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "server.h"
#include <fstream>
#include <map>
#include <iomanip>
#include <QDBusConnection>
#include <QDBusMetaType>
#include <QCoreApplication>
#include <QLocalSocket>
#include <QDir>
#include <QThread>
#include <QMutex>
#include <QDateTime>
#include <QtDebug>
#include "../sopluginloader.h"
#include "../../debugmessagehandler.h"
#include "marshalling.h"
#include "infoserverwrapper.h"

namespace LC
{
namespace DBus
{
	Server::Server ()
	: QObject ()
	{
		qInstallMessageHandler ([] (QtMsgType type, const QMessageLogContext& ctx, const QString& msg)
				{
					DebugHandler::Write (type, ctx, msg.toLocal8Bit ().constData (), DebugHandler::DWFNone);
				});

		const auto pid = QCoreApplication::applicationPid ();

		RegisterTypes ();

		auto sb = QDBusConnection::sessionBus ();
		const auto& serviceName = QString ("org.LeechCraft.Wrapper_%1").arg (pid);
		qDebug () << "registering service..." << sb.registerService (serviceName);
		qDebug () << "registering primary object..." << sb.registerObject ("/org/LeechCraft/Control",
				this, QDBusConnection::ExportAllContents);

		QLocalSocket socket;
		socket.connectToServer (QString ("lc_waiter_%1").arg (pid));
		qDebug () << "notifying master..." << socket.waitForConnected ();
	}

	bool Server::Load (const QString& path)
	{
		qDebug () << Q_FUNC_INFO << path;
		Loader_.reset (new Loaders::SOPluginLoader (path));
		if (!Loader_->Load ())
			return false;

		qDebug () << "done";
		QDBusConnection::sessionBus ().registerObject ("/org/LeechCraft/Plugin",
				Loader_->Instance (), QDBusConnection::ExportAllContents);

		if (auto ii = qobject_cast<IInfo*> (Loader_->Instance ()))
		{
			auto w = new InfoServerWrapper { ii };
			QDBusConnection::sessionBus ().registerObject ("/org/LeechCraft/Info",
					w, QDBusConnection::ExportAllContents);
		}

		return true;
	}

	bool Server::Unload (const QString&)
	{
		return Loader_->Unload ();
	}

	void Server::SetLcIconsPaths (const QStringList& paths)
	{
		for (const auto& path : paths)
			QDir::addSearchPath ("lcicons", path);
	}
}
}
