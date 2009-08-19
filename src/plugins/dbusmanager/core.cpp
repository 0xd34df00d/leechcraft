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
#include "adaptor.h"

using namespace LeechCraft;
using namespace LeechCraft::Plugins::DBusManager;

Core::Core ()
: NotificationManager_ (new NotificationManager)
{
	new Adaptor (this);

	QDBusConnection::sessionBus ().registerService ("org.LeechCraft.DBus");
	QDBusConnection::sessionBus ().registerObject ("/LeechCraft/Manager", this);
}

Core& Core::Instance ()
{
	static Core core;
	return core;
}

void Core::Release ()
{
	emit aboutToQuit ();
	Proxy_.reset ();
}

void Core::SetProxy (ICoreProxy_ptr proxy)
{
	Proxy_ = proxy;
	Proxy_->RegisterHook (HookSignature<HIDDownloadFinishedNotification>::Signature_t (
				boost::bind (&NotificationManager::HandleFinishedNotification,
				NotificationManager_.get (),
				_1,
				_2,
				_3)));
}

ICoreProxy_ptr Core::GetProxy () const
{
	return Proxy_;
}

QString Core::Greeter (const QString&)
{
	return "LeechCraft D-Bus connector development version";
}

QStringList Core::GetLoadedPlugins ()
{
	return QStringList ("Not implemented");
}

void Core::DumpError ()
{
	qDebug () << Q_FUNC_INFO
		<< Connection_->lastError ().message ();
}

