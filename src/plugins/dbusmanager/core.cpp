/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "core.h"
#include <boost/bind.hpp>
#include <QtDebug>
#include <QDBusError>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QApplication>
#include <QTimer>
#ifdef Q_WS_WIN32
#include <QProcess>
#endif
#include "generaladaptor.h"
#include "tasksadaptor.h"

using namespace LeechCraft;
using namespace LeechCraft::Plugins::DBusManager;

Core::Core ()
{
	NotificationManager_.reset (new NotificationManager);

	QTimer::singleShot (1500,
			this,
			SLOT (doDelayedInit ()));
#ifdef Q_WS_WIN32
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

QString Core::Greeter (const QString&)
{
	return tr ("LeechCraft D-Bus general interface");
}

bool Core::CouldHandle (const LeechCraft::DownloadEntity& e) const
{
	return NotificationManager_->CouldNotify (e);
}

void Core::Handle (const LeechCraft::DownloadEntity& e)
{
	NotificationManager_->HandleNotification (e);
}

void Core::DumpError ()
{
	qDebug () << Q_FUNC_INFO
		<< Connection_->lastError ().message ();
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
}

