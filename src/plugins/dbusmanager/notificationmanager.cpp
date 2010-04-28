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

	connect (Connection_.get (),
			SIGNAL (ActionInvoked (uint, QString)),
			this,
			SLOT (handleActionInvoked (uint, QString)));
}

bool NotificationManager::CouldNotify (const DownloadEntity& e) const
{
	return XmlSettingsManager::Instance ()->
			property ("UseNotifications").toBool () &&
		Connection_.get () &&
		Connection_->isValid () &&
		e.Mime_ == "x-leechcraft/notification" &&
		e.Additional_ ["Priority"].toInt () != PLog_;
}

void NotificationManager::HandleNotification (const DownloadEntity& e)
{
	if (!Connection_.get () ||
			!XmlSettingsManager::Instance ()->
				property ("UseNotifications").toBool ())
		return;

	Priority prio = static_cast<Priority> (e.Additional_ ["Priority"].toInt ());
	QString header = e.Entity_.toString ();
	QString text = e.Additional_ ["Text"].toString ();
	bool uus = e.Additional_ ["UntilUserSees"].toBool ();

	QStringList actions = e.Additional_ ["NotificationActions"].toStringList ();
	QStringList fmtActions;
	int i = 0;
	Q_FOREACH (QString action, actions)
		fmtActions << QString::number (i++) << action;

	if (prio == PLog_)
		return;

	QList<QVariant> arguments;
	arguments << header
		<< uint (0)
		<< QString ("leechcraft_main")
		<< QString ()
		<< text
		<< fmtActions
		<< QVariantMap ()
		<< (uus ?
				0 :
				Core::Instance ().GetProxy ()->GetSettingsManager ()->
				property ("FinishedDownloadMessageTimeout").toInt () * 1000);

	ActionData ad =
	{
		e.Additional_ ["HandlingObject"].value<QObject*> (),
		actions
	};

	QDBusPendingCall pending = Connection_->
			asyncCallWithArgumentList ("Notify", arguments);
	QDBusPendingCallWatcher *watcher =
		new QDBusPendingCallWatcher (pending, this);
	Watcher2AD_ [watcher] = ad;
	connect (watcher,
			SIGNAL (finished (QDBusPendingCallWatcher*)),
			this,
			SLOT (handleNotificationCallFinished (QDBusPendingCallWatcher*)));
}

void NotificationManager::handleNotificationCallFinished (QDBusPendingCallWatcher *w)
{
	QDBusPendingReply<uint> reply = *w;
	if (reply.isError ())
	{
		qWarning () << Q_FUNC_INFO
			<< reply.error ().name ()
			<< reply.error ().message ();
		return;
	}
	int id = reply.argumentAt<0> ();
	CallID2AD_ [id] = Watcher2AD_ [w];
	Watcher2AD_.remove (w);

	w->deleteLater ();
}

void NotificationManager::handleActionInvoked (uint id, QString action)
{
	const ActionData& ad = CallID2AD_ [id];
	int idx = action.toInt ();

	QMetaObject::invokeMethod (ad.Handler_,
			"notificationActionTriggered",
			Qt::QueuedConnection,
			Q_ARG (int, idx));

	CallID2AD_.remove (id);
}
