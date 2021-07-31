/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "kinotify.h"
#include <QIcon>
#include <util/util.h>
#include <util/xpc/util.h>
#include <util/xpc/notificationactionhandler.h>
#include <util/sll/void.h>
#include <util/sll/qtutil.h>
#include <util/threads/futures.h>
#include <interfaces/entitytesthandleresult.h>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "kinotifywidget.h"
#include "xmlsettingsmanager.h"
#include "fswinwatcher.h"

namespace LC
{
namespace Kinotify
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("kinotify");

		Proxy_ = proxy;

		SettingsDialog_.reset (new Util::XmlSettingsDialog ());
		SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"kinotifysettings.xml");

		connect (SettingsDialog_.get (),
				&Util::XmlSettingsDialog::pushButtonClicked,
				[this] (const QString& name)
				{
					if (name == "TestNotification"_ql)
						TestNotification ();
				});
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Kinotify";
	}

	QString Plugin::GetName () const
	{
		return "Kinotify";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Fancy Kinetic notifications.");
	}

	QIcon Plugin::GetIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		const bool could = e.Mime_ == "x-leechcraft/notification" &&
				!e.Additional_ ["Text"].toString ().isEmpty ();
		return could ?
				EntityTestHandleResult (EntityTestHandleResult::PHigh) :
				EntityTestHandleResult ();
	}

	namespace
	{
		QString GetPriorityIconName (Priority prio)
		{
			switch (prio)
			{
			case Priority::Warning:
				return "dialog-warning";
			case Priority::Critical:
				return "dialog-error";
			default:
				return "dialog-information";
			}
		}

		QFuture<Util::Void> OverridePixmap (KinotifyWidget *notificationWidget,
				const QVariant& notifVar, Priority prio, const ICoreProxy_ptr& proxy)
		{
			if (notifVar.canConvert<QPixmap> ())
			{
				auto pixmap = notifVar.value<QPixmap> ();
				if (!pixmap.isNull ())
				{
					notificationWidget->SetPixmap (std::move (pixmap));
					return Util::MakeReadyFuture<Util::Void> ({});
				}
			}
			else if (notifVar.canConvert<QImage> ())
			{
				auto image = notifVar.value<QImage> ();
				if (!image.isNull ())
				{
					notificationWidget->SetPixmap (QPixmap::fromImage (std::move (image)));
					return Util::MakeReadyFuture<Util::Void> ({});
				}
			}
			else if (notifVar.canConvert<Util::LazyNotificationPixmap_t> ())
			{
				const auto& maybePxFuture = notifVar.value<Util::LazyNotificationPixmap_t> () ();
				if (maybePxFuture)
					return Util::Sequence (notificationWidget, *maybePxFuture) >>
							[notificationWidget] (QImage image)
							{
								notificationWidget->SetPixmap (QPixmap::fromImage (std::move (image)));
								return Util::MakeReadyFuture<Util::Void> ({});
							};
			}

			const auto& icon = proxy->GetIconThemeManager ()->GetIcon (GetPriorityIconName (prio));
			notificationWidget->SetPixmap (icon.pixmap ({ 128, 128 }));
			return Util::MakeReadyFuture<Util::Void> ({});
		}
	}

	void Plugin::Handle (Entity e)
	{
		if (XmlSettingsManager::Instance ()->property ("RespectFullscreen").toBool () &&
				IsCurrentWindowFullScreen ())
			return;

		auto prio = e.Additional_ ["Priority"].value<Priority> ();

		const auto& sender = e.Additional_ ["org.LC.AdvNotifications.SenderID"].toString ();
		const auto& event = e.Additional_ ["org.LC.AdvNotifications.EventID"].toString ();
		const auto& notifyId = sender + event;

		auto sameIdPos = notifyId.isEmpty () ?
				ActiveNotifications_.end () :
				std::find_if (ActiveNotifications_.begin (), ActiveNotifications_.end (),
						[&notifyId] (KinotifyWidget *w) { return notifyId == w->GetID (); });

		const auto& header = e.Entity_.toString ();
		const auto& text = e.Additional_ ["Text"].toString ();
		const auto sameDataPos =
				std::find_if (ActiveNotifications_.begin (), ActiveNotifications_.end (),
						[&header, &text] (KinotifyWidget *w)
							{ return w->GetTitle () == header && w->GetBody () == text; });
		if (sameDataPos != ActiveNotifications_.end () && sameIdPos == ActiveNotifications_.end ())
			return;

		const auto defaultTimeout = XmlSettingsManager::Instance ()->
				property ("MessageTimeout").toInt () * 1000;
		const auto timeout = e.Additional_.value ("NotificationTimeout", defaultTimeout).toInt ();

		auto notificationWidget = new KinotifyWidget (timeout);
		notificationWidget->SetID (notifyId);

		QStringList actionsNames = e.Additional_ ["NotificationActions"].toStringList ();
		if (!actionsNames.isEmpty ())
		{
			if (!e.Additional_ ["HandlingObject"].canConvert<QObject_ptr> ())
				qWarning () << Q_FUNC_INFO
						<< "value is not QObject_ptr"
						<< e.Additional_ ["HandlingObject"];
			else
			{
				QObject_ptr actionObject = e.Additional_ ["HandlingObject"].value<QObject_ptr> ();
				notificationWidget->SetActions (actionsNames, actionObject);
			}
		}

		connect (notificationWidget,
				&QObject::destroyed,
				this,
				&Plugin::pushNotification);

		notificationWidget->SetContent (header, text);

		auto worker = [this, notificationWidget] (auto sameIdPos)
		{
			if (!ActiveNotifications_.size ())
				notificationWidget->PrepareNotification ();

			if (sameIdPos == ActiveNotifications_.end ())
				ActiveNotifications_ << notificationWidget;
			else if (sameIdPos == ActiveNotifications_.begin ())
			{
				auto oldNotify = *sameIdPos;
				std::advance (sameIdPos, 1);
				ActiveNotifications_.insert (sameIdPos, notificationWidget);
				oldNotify->deleteLater ();
			}
			else
			{
				(*sameIdPos)->deleteLater ();
				auto newPos = ActiveNotifications_.erase (sameIdPos);
				ActiveNotifications_.insert (newPos, notificationWidget);
			}
		};
		const auto& pxVar = e.Additional_ ["NotificationPixmap"];
		const auto& pxFuture = OverridePixmap (notificationWidget, pxVar, prio, Proxy_);
		if (pxFuture.isFinished ())
			worker (sameIdPos);
		else
			Util::Sequence (this, pxFuture) >>
					[worker, notifyId, this] (const auto&)
					{
						const auto sameIdPos = notifyId.isEmpty () ?
								ActiveNotifications_.end () :
								std::find_if (ActiveNotifications_.begin (), ActiveNotifications_.end (),
										[&notifyId] (KinotifyWidget *w) { return notifyId == w->GetID (); });
						worker (sameIdPos);
					};
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	void Plugin::TestNotification ()
	{
		auto e = Util::MakeNotification (tr ("Test notification"),
				tr ("This is a <em>test</em> notification body that will soon disappear."),
				Priority::Info);
		auto nah = new Util::NotificationActionHandler { e };
		nah->AddFunction (tr ("An action"), [] {});
		nah->AddFunction (tr ("Another action"), [] {});
		Handle (e);
	}

	void Plugin::pushNotification ()
	{
		if (!ActiveNotifications_.size ())
			return;

		ActiveNotifications_.removeFirst ();
		if (ActiveNotifications_.size ())
			ActiveNotifications_.first ()->PrepareNotification ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_kinotify, LC::Kinotify::Plugin);
