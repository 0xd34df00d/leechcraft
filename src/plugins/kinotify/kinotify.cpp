/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "kinotify.h"
#include <QIcon>
#include <util/xpc/util.h>
#include <util/xpc/notificationactionhandler.h>
#include <util/sll/void.h>
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>
#include <interfaces/entityconstants.h>
#include <interfaces/an/entityfields.h>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "kinotifywidget.h"
#include "xmlsettingsmanager.h"
#include "fswinwatcher.h"

namespace LC::Kinotify
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		SettingsDialog_ = std::make_shared<Util::XmlSettingsDialog> ();
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				QStringLiteral ("kinotifysettings.xml"));

		connect (SettingsDialog_.get (),
				&Util::XmlSettingsDialog::pushButtonClicked,
				this,
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
		return QStringLiteral ("Kinotify");
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Fancy Kinetic notifications.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		const bool could = e.Mime_ == LC::Mimes::Notification &&
				!e.Additional_ [LC::EF::Text].toString ().isEmpty ();
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
				return QStringLiteral ("dialog-warning");
			case Priority::Critical:
				return QStringLiteral ("dialog-error");
			default:
				return QStringLiteral ("dialog-information");
			}
		}

		Util::ContextTask<void> OverridePixmap (KinotifyWidget *notificationWidget, const QVariant& notifVar, Priority prio)
		{
			if (auto pixmap = notifVar.value<QPixmap> ();
				!pixmap.isNull ())
			{
				notificationWidget->SetPixmap (std::move (pixmap));
				co_return;
			}

			if (auto image = notifVar.value<QImage> ();
				!image.isNull ())
			{
				notificationWidget->SetPixmap (QPixmap::fromImage (std::move (image)));
				co_return;
			}

			const auto& icon = GetProxyHolder ()->GetIconThemeManager ()->GetIcon (GetPriorityIconName (prio));
			const auto prioritySize = 128;
			notificationWidget->SetPixmap (icon.pixmap ({ prioritySize, prioritySize }));

			if (const auto pxTaskGetter = notifVar.value<Util::LazyNotificationPixmap_t> ())
			{
				co_await Util::AddContext { Util::CoroContext::Of (*notificationWidget) };
				if (const auto px = co_await pxTaskGetter ())
					notificationWidget->SetPixmap (QPixmap::fromImage (*px));
			}
		}
	}

	void Plugin::Handle (Entity e)
	{
		if (XmlSettingsManager::Instance ().property ("RespectFullscreen").toBool () &&
				IsCurrentWindowFullScreen ())
			return;

		auto prio = e.Additional_ [LC::EF::Priority].value<Priority> ();

		const auto& sender = e.Additional_ [LC::AN::EF::SenderID].toString ();
		const auto& event = e.Additional_ [LC::AN::EF::EventID].toString ();
		const auto& notifyId = sender + event;

		auto sameIdPos = notifyId.isEmpty () ?
				ActiveNotifications_.end () :
				std::find_if (ActiveNotifications_.begin (), ActiveNotifications_.end (),
						[&notifyId] (KinotifyWidget *w) { return notifyId == w->GetID (); });

		const auto& header = e.Entity_.toString ();
		const auto& text = e.Additional_ [LC::EF::Text].toString ();
		const auto sameDataPos =
				std::find_if (ActiveNotifications_.begin (), ActiveNotifications_.end (),
						[&header, &text] (KinotifyWidget *w)
							{ return w->GetTitle () == header && w->GetBody () == text; });
		if (sameDataPos != ActiveNotifications_.end () && sameIdPos == ActiveNotifications_.end ())
			return;

		const auto defaultTimeout = XmlSettingsManager::Instance ().property ("MessageTimeout").toInt () * 1000;
		const auto timeout = e.Additional_.value (LC::EF::NotificationTimeout, defaultTimeout).toInt ();

		auto notificationWidget = new KinotifyWidget (timeout);
		notificationWidget->SetID (notifyId);

		auto actionsNames = e.Additional_ [LC::EF::NotificationActions].toStringList ();
		if (!actionsNames.isEmpty ())
		{
			const auto& handlingObjectVar = e.Additional_ [LC::EF::HandlingObject];
			if (!handlingObjectVar.canConvert<QObject_ptr> ())
				qWarning () << Q_FUNC_INFO
						<< "value is not QObject_ptr"
						<< handlingObjectVar;
			else
				notificationWidget->SetActions (actionsNames, handlingObjectVar.value<QObject_ptr> ());
		}

		connect (notificationWidget,
				&QObject::destroyed,
				this,
				[this, notificationWidget]
				{
					const auto wasDisplayed = ActiveNotifications_.value (0) == notificationWidget;
					ActiveNotifications_.removeOne (notificationWidget);
					if (wasDisplayed && !ActiveNotifications_.isEmpty ())
						ActiveNotifications_.first ()->PrepareNotification ();
				});

		notificationWidget->SetContent (header, text);
		OverridePixmap (notificationWidget, e.Additional_ [LC::EF::NotificationPixmap], prio);

		if (ActiveNotifications_.isEmpty ())
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
}

LC_EXPORT_PLUGIN (leechcraft_kinotify, LC::Kinotify::Plugin);
