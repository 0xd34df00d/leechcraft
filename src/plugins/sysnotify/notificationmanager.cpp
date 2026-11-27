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
#include <QDBusServiceWatcher>
#include <QIcon>
#include <QDBusMetaType>
#include <QtDebug>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>
#include <util/threads/coro/dbus.h>
#include <util/xpc/util.h>
#include "bodymarkup.h"
#include "imagehint.h"

namespace LC
{
namespace Sysnotify
{
	NotificationManager::NotificationManager (QObject *parent)
	: QObject { parent }
	, Server_
	{
		{
			.Service = "org.freedesktop.Notifications"_qs,
			.Path = "/org/freedesktop/Notifications"_qs,
			.Interface = "org.freedesktop.Notifications"_qs,
		}
	}
	{
		qDBusRegisterMetaType<ImageHint> ();

		const auto watcher = new QDBusServiceWatcher
		{
			Server_.Service,
			Server_.Conn,
			QDBusServiceWatcher::WatchForOwnerChange,
			this
		};
		connect (watcher,
				&QDBusServiceWatcher::serviceOwnerChanged,
				this,
				[this] (const QString&, const QString&, const QString& newOwner)
				{
					HandleServerChanged (newOwner);
				});
		HandleServerChanged (Server_.Conn.interface ()->serviceOwner (Server_.Service).value ());

		Server_.Connect ("ActionInvoked"_qs, this, &NotificationManager::HandleActionInvoked);
		Server_.Connect ("NotificationClosed"_qs, this, &NotificationManager::HandleNotificationClosed);
	}

	bool NotificationManager::CouldNotify (const Entity& e) const
	{
		return !UsableServer_.isEmpty () &&
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

		const auto server = UsableServer_;
		if (server.isEmpty ())
			co_return;

		if (!Capabilities_)
		{
			const auto eitherCaps = co_await Server_.Call<QStringList> ("GetCapabilities"_qs);
			if (UsableServer_ != server)
				co_return;

			Util::Visit (eitherCaps,
					[this] (const QStringList& caps) { Capabilities_ = caps; },
					[] (const QDBusError& err) { qWarning () << "unable to query the capabilities:" << err; });
		}

		const auto& caps = Capabilities_.value_or (QStringList {});
		auto actions = e.Additional_ ["NotificationActions"].toStringList ();
		if (!caps.contains ("actions"))
			actions.clear ();

		const auto& header = e.Entity_.toString ();
		const auto& text = Util::GetNotificationText (e);
		const auto& body = caps.contains ("body-markup") ?
				ToBodyMarkup (text, caps.contains ("body-hyperlinks")) :
				Util::ToPlainText (text);
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

		const auto eitherId = co_await Server_.Call<uint> ("Notify"_qs,
				header,
				0u,
				"leechcraft_main"_qs,
				QString {},
				body,
				fmtActions,
				hints,
				timeout);
		const auto id = co_await Util::WithHandler (eitherId,
				[] (const QDBusError& err) { qWarning () << "unable to show the notification:" << err; });
		if (UsableServer_ == server)
			CallID2AD_ [id] = { e, e.Additional_ ["HandlingObject"].value<QObject_ptr> (), actions };
	}

	void NotificationManager::HandleServerChanged (const QString& server)
	{
		CallID2AD_.clear ();
		UsableServer_ = server;
		Capabilities_.reset ();
		if (!server.isEmpty ())
			CheckServer (server);
	}

	Util::ContextTask<void> NotificationManager::CheckServer (QString server)
	{
		co_await Util::AddContext { CoroContext_ };

		const auto reject = [&]
		{
			if (UsableServer_ == server)
				UsableServer_.clear ();
		};

		const auto eitherInfo = co_await Server_.Call<QString, QString, QString, QString> ("GetServerInformation"_qs);
		const auto& [implementation, vendor, serverVersion, versionString] = co_await Util::WithHandler (eitherInfo,
				[&] (const QDBusError& err)
				{
					qWarning () << "unable to query the server information:" << err;
					reject ();
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
			reject ();
	}

	void NotificationManager::HandleActionInvoked (uint id, const QString& action)
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

	void NotificationManager::HandleNotificationClosed (uint id)
	{
		CallID2AD_.remove (id);
	}
}
}
