/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "systemtrayhandler.h"
#include <QApplication>
#include <QMenu>
#include <QPainter>
#include <QSystemTrayIcon>
#include <interfaces/structures.h>
#include <interfaces/entityconstants.h>
#include <interfaces/an/constants.h>
#include <interfaces/an/entityfields.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ientitymanager.h>
#include <util/xpc/util.h>
#include <util/gui/geometry.h>
#include <util/gui/unhoverdeletemixin.h>
#include <util/gui/util.h>
#include <util/sll/containerconversions.h>
#include <util/sll/qtutil.h>
#include <util/threads/coro/future.h>
#include <util/threads/coro.h>
#include "generalhandler.h"
#include "xmlsettingsmanager.h"
#include "visualnotificationsview.h"

namespace LC::AdvancedNotifications
{
	SystemTrayHandler::~SystemTrayHandler ()
	{
		for (auto icon : Category2Icon_)
		{
			icon->hide ();
			delete icon;
		}
	}

	NotificationMethod SystemTrayHandler::GetHandlerMethod () const
	{
		return NMTray;
	}

	void SystemTrayHandler::Handle (const Entity& e, const NotificationRule&)
	{
		const auto& cat = e.Additional_ [AN::EF::EventCategory].toString ();
		const auto& senderId = e.Additional_ [AN::EF::SenderID].toByteArray ();
		const auto& eventId = e.Additional_ [AN::EF::EventID].toString ();

		if (cat == AN::CatEventCancel)
		{
			if (Events_.remove ({ senderId, eventId }))
				RebuildState ();
			return;
		}

		const EventKey key { senderId, eventId };

		PrepareSysTrayIcon (cat);
		PrepareLCTrayAction (cat);
		auto& eventData = [&] () -> EventData&
		{
			if (!Events_.contains (key))
			{
				EventData data;
				data.SenderId_ = senderId;
				data.EventId_ = eventId;
				data.Count_ = 0;
				data.Category_ = cat;
				data.VisualPath_ = e.Additional_ [AN::EF::VisualPath].toStringList ();
				data.HandlingObject_ = e.Additional_ [EF::HandlingObject].value<QObject_ptr> ();
				data.Actions_ = e.Additional_ [EF::NotificationActions].toStringList ();
				data.Canceller_ = Util::MakeANCancel (e);
				Events_ [key] = data;
			}
			return Events_ [key];
		} ();

		if (const int delta = e.Additional_.value (AN::EF::DeltaCount, 0).toInt ())
			eventData.Count_ += delta;
		else
			eventData.Count_ = e.Additional_.value (AN::EF::Count, 1).toInt ();
		eventData.ExtendedText_ = e.Additional_ [AN::EF::ExtendedText].toString ();
		eventData.FullText_ = e.Additional_ [AN::EF::FullText].toString ();

		ExtractPixmap (eventData, e);

		RebuildState ();
	}

	namespace
	{
		QPixmap GetDefaultPixmap (Priority prio)
		{
			QLatin1String mi;
			switch (prio)
			{
			case Priority::Info:
				mi = "information"_ql;
				break;
			case Priority::Warning:
				mi = "warning"_ql;
				break;
			case Priority::Critical:
				mi = "error"_ql;
				break;
			}

			constexpr auto size = 64;
			return GetProxyHolder ()->GetIconThemeManager ()->GetIcon ("dialog-" + mi).pixmap ({ size, size });
		}

		std::optional<QPixmap> ExtractPixmapSync (const QVariant& pxVar)
		{
			if (pxVar.canConvert<QPixmap> ())
				return pxVar.value<QPixmap> ();
			if (pxVar.canConvert<QImage> ())
				return QPixmap::fromImage (pxVar.value<QImage> ());

			return {};
		}
	}

	Util::ContextTask<void> SystemTrayHandler::ExtractPixmap (EventData& eventData, const Entity& e)
	{
		const auto& pxVar = e.Additional_ [EF::NotificationPixmap];

		if (const auto px = ExtractPixmapSync (pxVar))
		{
			eventData.Pixmap_ = *px;
			co_return;
		}

		eventData.Pixmap_ = GetDefaultPixmap (e.Additional_ [EF::Priority].value<Priority> ());

		if (const auto& lazyPxGetter = pxVar.value<Util::LazyNotificationPixmap_t> ();
			const auto& maybeLazy = lazyPxGetter ())
		{
			co_await Util::AddContextObject { *this };
			const auto& px = co_await *maybeLazy;
			const auto pos = Events_.find ({ eventData.SenderId_, eventData.EventId_ });
			if (pos != Events_.end ())
			{
				pos->Pixmap_ = QPixmap::fromImage (px);
				RebuildState ();
			}
		}
	}

	namespace
	{
		QIcon GetIconForCategory (const QString& cat)
		{
			static const QMap<QString, QString> cat2iconName
			{
				{ AN::CatDownloads, "folder-downloads" },
				{ AN::CatIM, "mail-unread-new" },
				{ AN::CatOrganizer, "view-calendar" },
				{ AN::CatGeneric, "preferences-desktop-notification-bell" },
				{ AN::CatPackageManager, "system-software-update" },
				{ AN::CatMediaPlayer, "applications-multimedia" },
				{ AN::CatTerminal, "utilities-terminal" },
				{ AN::CatNews, "view-pim-news" },
			};

			auto name = cat2iconName.value (cat);
			if (name.isEmpty ())
			{
				qWarning () << Q_FUNC_INFO
						<< "no icon for category"
						<< cat;
				name = "dialog-information"_ql;
			}

			return GetProxyHolder ()->GetIconThemeManager ()->GetIcon (name);
		}

		void ShowVNV (VisualNotificationsView *view, const QList<EventData>& events)
		{
			if (!view->isVisible ())
			{
				view->SetEvents (events);
				view->move (Util::FitRectScreen (QCursor::pos (), view->size ()));
			}
			view->setVisible (!view->isVisible ());
		}
	}

	VisualNotificationsView* SystemTrayHandler::CreateVisualNotificationView ()
	{
		const auto vnv = new VisualNotificationsView { GetProxyHolder () };
		connect (vnv,
				&VisualNotificationsView::actionTriggered,
				this,
				&SystemTrayHandler::HandleActionTriggered);
		connect (vnv,
				&VisualNotificationsView::dismissEvent,
				this,
				&SystemTrayHandler::DismissNotification);
		return vnv;
	}

	void SystemTrayHandler::PrepareSysTrayIcon (const QString& category)
	{
#ifndef Q_OS_MACOS
		if (Category2Icon_.contains (category))
			return;

		const auto trayIcon = new QSystemTrayIcon (Util::FixupTrayIcon (GetIconForCategory (category)));
		trayIcon->setContextMenu (new QMenu ());
		Category2Icon_ [category] = trayIcon;

		connect (trayIcon,
				&QSystemTrayIcon::activated,
				this,
				[this, trayIcon] (QSystemTrayIcon::ActivationReason reason)
				{
					if (reason == QSystemTrayIcon::Trigger)
						ShowVNV (Icon2NotificationView_ [trayIcon], EventsForIcon_ [trayIcon]);
				});

		const auto vnv = CreateVisualNotificationView ();
		Icon2NotificationView_ [trayIcon] = vnv;

		if (XmlSettingsManager::Instance ().property ("HideOnHoverOut").toBool ())
			new Util::UnhoverDeleteMixin (vnv, SLOT (hide ()));
#endif
	}

	void SystemTrayHandler::PrepareLCTrayAction (const QString& category)
	{
		if (Category2Action_.contains (category))
			return;

		const auto action = new QAction (GetIconForCategory (category), category, this);
		Category2Action_ [category] = action;

		connect (action,
				&QAction::triggered,
				this,
				[this, action]
				{
					ShowVNV (Action2NotificationView_ [action], EventsForAction_ [action]);
				});

		emit gotActions ({ action }, ActionsEmbedPlace::LCTray);

		const auto vnv = CreateVisualNotificationView ();
		Action2NotificationView_ [action] = vnv;

		if (XmlSettingsManager::Instance ().property ("HideOnHoverOut").toBool ())
			new Util::UnhoverDeleteMixin (vnv, SLOT (hide ()));
	}

	void SystemTrayHandler::UpdateMenu (QMenu *menu, const EventKey& event, const EventData& data)
	{
		for (const auto& pathItem : data.VisualPath_)
			menu = menu->addMenu (pathItem);

		if (!data.Pixmap_.isNull ())
			menu->setIcon (data.Pixmap_);
		menu->setToolTip (data.ExtendedText_);

		int actionIdx = 0;
		for (const auto& actionName : data.Actions_)
		{
			const auto action = menu->addAction (actionName);
			connect (action,
					&QAction::triggered,
					this,
					[this, event, idx = actionIdx++] { HandleActionTriggered (event, idx); },
					Qt::QueuedConnection);
		}

		const auto dismiss = menu->addAction (tr ("Dismiss"));
		connect (dismiss,
				&QAction::triggered,
				this,
				[this, event] { DismissNotification (event); },
				Qt::QueuedConnection);

		menu->addSeparator ();
		menu->addAction (data.ExtendedText_)->setEnabled (false);
	}

	void SystemTrayHandler::RebuildState ()
	{
		auto icons2hide = Util::AsSet (Category2Icon_);
		for (const auto icon : icons2hide)
			icon->contextMenu ()->clear ();

		auto actsDel = Util::AsSet (Category2Action_);

		EventsForIcon_.clear ();
		EventsForAction_.clear ();

		QSet<QSystemTrayIcon*> visibleIcons;
		QSet<QAction*> actsUpd;

		for (const auto& [event, data] : Util::Stlize (Events_))
		{
			const auto icon = Category2Icon_.value (data.Category_);
			if (icon)
			{
				icons2hide.remove (icon);
				visibleIcons << icon;
			}

			QAction *action = Category2Action_ [data.Category_];
			actsDel.remove (action);
			actsUpd << action;

			EventsForAction_ [action] << data;

			if (icon)
			{
				EventsForIcon_ [icon] << data;
				UpdateMenu (icon->contextMenu (), event, data);
			}
		}

		for (const auto icon : Category2Icon_)
		{
			const auto view = Icon2NotificationView_ [icon];
			if (!view->isVisible ())
				continue;

			const auto& events = EventsForIcon_ [icon];
			view->SetEvents (events);
			if (events.isEmpty ())
				view->hide ();
		}

		for (const auto action : Category2Action_)
		{
			const auto view = Action2NotificationView_ [action];
			if (!view->isVisible ())
				continue;

			const auto& events = EventsForAction_ [action];
			view->SetEvents (events);
			if (events.isEmpty ())
				view->hide ();
		}

		for (const auto icon : visibleIcons)
		{
			if (!icon->isVisible ())
				icon->show ();
			UpdateSysTrayIcon (icon);
		}

		for (const auto action : actsUpd)
			UpdateTrayAction (action);

		for (const auto icon : icons2hide)
			icon->hide ();

		for (const auto action : actsDel)
		{
			Category2Action_.remove (Category2Action_.key (action));
			EventsForAction_.remove (action);
			delete Action2NotificationView_.take (action);
		}
		qDeleteAll (actsDel);
	}

	template<typename T>
	void SystemTrayHandler::UpdateIcon (T iconable, const QString& category)
	{
		QIcon icon = GetIconForCategory (category);
		if (!XmlSettingsManager::Instance ()
				.property ("EnableCounter." + category.toLatin1 ()).toBool ())
		{
			iconable->setIcon (icon);
			return;
		}

		int eventCount = std::accumulate (Events_.begin (), Events_.end (), 0,
				[&category] (int acc, const EventData& event)
				{
					return event.Category_ == category ?
							acc + event.Count_ :
							acc;
				});

		const auto& palette = QGuiApplication::palette ();

		auto font = QGuiApplication::font ();
		font.setItalic (true);

		QIcon withText;
		for (const auto& size : icon.availableSizes ())
		{
			const auto& px = icon.pixmap (size);
			const auto& overlaid = Util::DrawOverlayText (px,
					QString::number (eventCount), font,
					QPen (palette.color (QPalette::ButtonText)),
					QBrush (palette.color (QPalette::Button)));
			withText.addPixmap (overlaid);
		}

		iconable->setIcon (withText);
	}

	void SystemTrayHandler::UpdateSysTrayIcon (QSystemTrayIcon *trayIcon)
	{
		UpdateIcon (trayIcon, Category2Icon_.key (trayIcon));
	}

	void SystemTrayHandler::UpdateTrayAction (QAction *action)
	{
		UpdateIcon (action, Category2Action_.key (action));
	}

	void SystemTrayHandler::HandleActionTriggered (const EventKey& event, int index)
	{
		if (!Events_.contains (event))
		{
			qWarning () << "no such event" << event.SenderId_ << event.EventId_;
			return;
		}

		QMetaObject::invokeMethod (Events_ [event].HandlingObject_.get (),
				"notificationActionTriggered",
				Qt::QueuedConnection,
				Q_ARG (int, index));
	}

	void SystemTrayHandler::DismissNotification (const EventKey& key)
	{
		if (!Events_.contains (key))
			return;

		const auto canceller = Events_.value (key).Canceller_;
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (canceller);
	}
}
