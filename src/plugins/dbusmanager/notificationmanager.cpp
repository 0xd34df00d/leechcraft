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

#include "notificationmanager.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QApplication>
#include <QIcon>
#include <QtDebug>
#include "core.h"
#include "xmlsettingsmanager.h"

using namespace LeechCraft;
using namespace LeechCraft::Plugins::DBusManager;

NotificationManager::NotificationManager (QObject *parent)
: QObject (parent)
{
	if (!QDBusConnection::sessionBus ().interface ()->
			isServiceRegistered ("org.freedesktop.Notifications"))
	{
		qWarning () << Q_FUNC_INFO
			<< QDBusConnection::sessionBus ().interface ()->registeredServiceNames ().value ();
		return;
	}

	Connection_.reset (new QDBusInterface ("org.freedesktop.Notifications",
				"/org/freedesktop/Notifications"));
	if (!Connection_->isValid ())
	{
		qWarning () << Q_FUNC_INFO
			<< Connection_->lastError ();
	}
}

void NotificationManager::HandleFinishedNotification (IHookProxy_ptr proxy,
		Notification *n, bool show)
{
	if (!Connection_.get () ||
			!show ||
			!XmlSettingsManager::Instance ()->
				property ("UseNotifications").toBool ())
		return;

	if (n->Priority_ == Notification::PLog_)
		return;

	QList<QVariant> arguments;
	arguments << n->Header_
		<< uint (0)
		<< QString ("leechcraft_main")
		<< QString ()
		<< n->Text_
		<< QStringList ()
		<< QVariantMap ()
		<< (n->UntilUserSees_ ? 0
				: Core::Instance ().GetProxy ()->GetSettingsManager ()->
				property ("FinishedDownloadMessageTimeout").toInt () * 1000);

	if (Connection_->callWithArgumentList (QDBus::NoBlock,
			"Notify", arguments).type () == QDBusMessage::ErrorMessage)
		qWarning () << Q_FUNC_INFO
			<< Connection_->lastError ();
	else
		proxy->CancelDefault ();
}

