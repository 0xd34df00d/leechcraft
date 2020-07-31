/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "notificationmanager.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QIcon>
#include <QDBusMetaType>
#include <QtDebug>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include "imagehint.h"

namespace LC
{
namespace Sysnotify
{
	NotificationManager::NotificationManager (QObject *parent)
	: QObject { parent }
	{
		if (!QDBusConnection::sessionBus ().interface ()->
				isServiceRegistered ("org.freedesktop.Notifications"))
		{
			qWarning () << Q_FUNC_INFO
				<< QDBusConnection::sessionBus ().interface ()->registeredServiceNames ().value ();
			return;
		}

		qDBusRegisterMetaType<ImageHint> ();

		Connection_.reset (new QDBusInterface ("org.freedesktop.Notifications",
					"/org/freedesktop/Notifications"));
		if (!Connection_->isValid ())
			qWarning () << Q_FUNC_INFO
					<< Connection_->lastError ();

		auto pendingSI = Connection_->asyncCall ("GetServerInformation");
		connect (new QDBusPendingCallWatcher { pendingSI, this },
				SIGNAL (finished (QDBusPendingCallWatcher*)),
				this,
				SLOT (handleGotServerInfo (QDBusPendingCallWatcher*)));

		connect (Connection_.get (),
				SIGNAL (ActionInvoked (uint, QString)),
				this,
				SLOT (handleActionInvoked (uint, QString)));
		connect (Connection_.get (),
				SIGNAL (NotificationClosed (uint, uint)),
				this,
				SLOT (handleNotificationClosed (uint, uint)));
	}

	bool NotificationManager::CouldNotify (const Entity& e) const
	{
		return Connection_.get () &&
				Connection_->isValid () &&
				e.Mime_ == "x-leechcraft/notification" &&
				!e.Additional_ ["Text"].toString ().isEmpty ();
	}

	void NotificationManager::HandleNotification (const Entity& e)
	{
		if (!Connection_.get ())
			return;

		const auto& actions = e.Additional_ ["NotificationActions"].toStringList ();
		if (actions.isEmpty ())
		{
			DoNotify (e, false);
			return;
		}

		const auto& pending = Connection_->asyncCall ("GetCapabilities");
		const auto watcher = new QDBusPendingCallWatcher { pending, this };
		Watcher2CapCheck_ [watcher] = { e };
		connect (watcher,
				SIGNAL (finished (QDBusPendingCallWatcher*)),
				this,
				SLOT (handleCapCheckCallFinished (QDBusPendingCallWatcher*)));
	}

	namespace
	{
		QImage ToImage (const QVariant& imageVar)
		{
			if (imageVar.canConvert<QPixmap> ())
				return imageVar.value<QPixmap> ()
						.toImage ().convertToFormat (QImage::Format_ARGB32);
			else if (imageVar.canConvert<QImage> ())
				return imageVar.value<QImage> ().convertToFormat (QImage::Format_ARGB32);

			return {};
		}
	}

	void NotificationManager::DoNotify (const Entity& e, bool hasActions)
	{
		const auto& header = e.Entity_.toString ();
		const auto& text = e.Additional_ ["Text"].toString ();
		bool uus = e.Additional_ ["UntilUserSees"].toBool ();

		QStringList fmtActions;
		QStringList actions;
		if (hasActions)
		{
			actions = e.Additional_ ["NotificationActions"].toStringList ();
			int i = 0;
			for (const auto& action : actions)
				fmtActions << QString::number (i++) << action;
		}

		int timeout = 0;
		if (!uus)
			timeout = 5000;

		QVariantMap hints;
		const auto& image = ToImage (e.Additional_ ["NotificationPixmap"]);
		if (!image.isNull ())
		{
			const auto& imageVar = QVariant::fromValue<ImageHint> (image);
			hints ["image_data"] = imageVar;
			hints ["icon_data"] = imageVar;
			hints ["image-data"] = imageVar;
		}

		QList<QVariant> arguments
		{
			header,
			static_cast<uint> (0),
			QString { "leechcraft_main" },
			QString {},
			text,
			fmtActions,
			hints,
			timeout
		};

		ActionData ad
		{
			e,
			e.Additional_ ["HandlingObject"].value<QObject_ptr> (),
			actions
		};

		const auto& pending = Connection_->asyncCallWithArgumentList ("Notify", arguments);
		const auto watcher = new QDBusPendingCallWatcher { pending, this };
		Watcher2AD_ [watcher] = ad;
		connect (watcher,
				SIGNAL (finished (QDBusPendingCallWatcher*)),
				this,
				SLOT (handleNotificationCallFinished (QDBusPendingCallWatcher*)));
	}

	void NotificationManager::handleGotServerInfo (QDBusPendingCallWatcher *w)
	{
		w->deleteLater ();

		QDBusPendingReply<QString, QString, QString, QString> reply = *w;
		if (reply.isError ())
		{
			qWarning () << Q_FUNC_INFO
					<< reply.error ().name ()
					<< reply.error ().message ();
			Connection_.reset ();
			return;
		}

		const auto& implementation = reply.argumentAt<0> ();
		const auto& vendor = reply.argumentAt<1> ();
		auto versionString = reply.argumentAt<3> ();
		qDebug () << Q_FUNC_INFO
				<< "using"
				<< implementation
				<< vendor
				<< reply.argumentAt<2> ()
				<< versionString;

		const auto& versionSplit = versionString.splitRef ('.', Qt::SkipEmptyParts);
		if (versionSplit.size () == 2)
			Version_ = std::make_tuple (versionSplit.value (0).toInt (),
					versionSplit.value (1).toInt ());

		if (vendor == "LeechCraft")
			Connection_.reset ();
		else if (implementation == "Plasma")
			IgnoreTimeoutCloses_ = true;						// KDE is shit violating specs.
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

	void NotificationManager::handleCapCheckCallFinished (QDBusPendingCallWatcher *w)
	{
		QDBusPendingReply<QStringList> reply = *w;
		if (reply.isError ())
		{
			qWarning () << Q_FUNC_INFO
				<< "failed to handle notification, failed to query caps:"
				<< reply.error ().name ()
				<< reply.error ().message ();
			return;
		}
		const auto& caps = reply.argumentAt<0> ();
		bool hasActions = caps.contains ("actions");
		DoNotify (Watcher2CapCheck_.take (w).Entity_, hasActions);
	}

	void NotificationManager::handleActionInvoked (uint id, QString action)
	{
		const auto& ad = CallID2AD_.take (id);
		if (!ad.Handler_)
		{
			qWarning () << Q_FUNC_INFO
					<< "handler already destroyed";
			return;
		}

		const auto idx = action.toInt ();

		QMetaObject::invokeMethod (ad.Handler_.get (),
				"notificationActionTriggered",
				Qt::QueuedConnection,
				Q_ARG (int, idx));
	}

	void NotificationManager::handleNotificationClosed (uint id, uint reason)
	{
		if (IgnoreTimeoutCloses_ && reason == 1)
			return;

		CallID2AD_.remove (id);
	}
}
}
