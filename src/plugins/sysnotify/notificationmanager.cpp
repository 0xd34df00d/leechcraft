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
#include <util/threads/coro.h>
#include <util/threads/coro/dbus.h>
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
					"/org/freedesktop/Notifications",
					"org.freedesktop.Notifications"));
		if (!Connection_->isValid ())
			qWarning () << Q_FUNC_INFO
					<< Connection_->lastError ();

		CheckServer ();

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

	Util::ContextTask<void> NotificationManager::HandleNotification (const Entity e)
	{
		co_await Util::AddContext { CoroContext_ };

		if (!Connection_)
			co_return;

		auto actions = e.Additional_ ["NotificationActions"].toStringList ();
		if (!actions.isEmpty ())
		{
			const auto& pending = Connection_->asyncCall ("GetCapabilities");
			const auto eitherCaps = co_await Util::Typed<QStringList> (pending);
			const auto& caps = co_await Util::WithHandler (eitherCaps,
					[] (const QDBusError& err) { qWarning () << "unable to query the capabilities:" << err; });
			if (!Connection_)
				co_return;

			if (!caps.contains ("actions"))
				actions.clear ();
		}

		const auto& header = e.Entity_.toString ();
		const auto& text = e.Additional_ ["Text"].toString ();
		bool uus = e.Additional_ ["UntilUserSees"].toBool ();

		QStringList fmtActions;
		for (int i = 0; i < actions.size (); ++i)
			fmtActions << QString::number (i) << actions.at (i);

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

		const auto& pending = Connection_->asyncCallWithArgumentList ("Notify", arguments);
		const auto eitherId = co_await Util::Typed<uint> (pending);
		const auto id = co_await Util::WithHandler (eitherId,
				[] (const QDBusError& err) { qWarning () << "unable to show the notification:" << err; });
		CallID2AD_ [id] = { e, e.Additional_ ["HandlingObject"].value<QObject_ptr> (), actions };
	}

	Util::ContextTask<void> NotificationManager::CheckServer ()
	{
		co_await Util::AddContext { CoroContext_ };

		const auto& pending = Connection_->asyncCall ("GetServerInformation");
		const auto eitherInfo = co_await Util::Typed<QString, QString, QString, QString> (pending);
		const auto& [implementation, vendor, serverVersion, versionString] = co_await Util::WithHandler (eitherInfo,
				[this] (const QDBusError& err)
				{
					qWarning () << "unable to query the server information:" << err;
					Connection_.reset ();
				});
		qDebug () << "using"
				<< implementation
				<< vendor
				<< serverVersion
				<< versionString;

		const auto& versionSplit = QStringView { versionString }.split ('.', Qt::SkipEmptyParts);
		if (versionSplit.size () == 2)
			Version_ = std::make_tuple (versionSplit.value (0).toInt (),
					versionSplit.value (1).toInt ());

		if (vendor == "LeechCraft")
			Connection_.reset ();
		else if (implementation == "Plasma")
			IgnoreTimeoutCloses_ = true;						// KDE is shit violating specs.
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

		QMetaObject::invokeMethod (this,
				[handler = ad.Handler_, idx = action.toInt ()]
				{
					QMetaObject::invokeMethod (handler.get (),
							"notificationActionTriggered",
							Q_ARG (int, idx));
				},
				Qt::QueuedConnection);
	}

	void NotificationManager::handleNotificationClosed (uint id, uint reason)
	{
		if (IgnoreTimeoutCloses_ && reason == 1)
			return;

		CallID2AD_.remove (id);
	}
}
}
