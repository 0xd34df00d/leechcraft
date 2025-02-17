/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "systemtrayhandler.h"
#include <QMenu>
#include <QPainter>
#include <QApplication>
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
#include <util/threads/futures.h>
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

	namespace
	{
		QFuture<QPixmap> GetPixmap (const Entity& e)
		{
			const auto& pxVar = e.Additional_ [EF::NotificationPixmap];

			if (pxVar.canConvert<QPixmap> ())
				return Util::MakeReadyFuture (pxVar.value<QPixmap> ());

			if (pxVar.canConvert<QImage> ())
				return Util::MakeReadyFuture (QPixmap::fromImage (pxVar.value<QImage> ()));

			const auto prio = e.Additional_ [EF::Priority].value<Priority> ();
			auto getDefault = [prio]
			{
				QString mi;
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

				const auto iconSize = 64;
				const auto& pixmap = GetProxyHolder ()->GetIconThemeManager ()->
						GetIcon ("dialog-" + mi).pixmap ({ iconSize, iconSize });
				return Util::MakeReadyFuture (pixmap);
			};

			if (pxVar.canConvert<Util::LazyNotificationPixmap_t> ())
			{
				const auto& maybeLazy = pxVar.value<Util::LazyNotificationPixmap_t> () ();
				if (!maybeLazy)
					return getDefault ();

				return Util::Sequence (nullptr, *maybeLazy) >>
						[] (const QImage& img) { return Util::MakeReadyFuture (QPixmap::fromImage (img)); };
			}

			return getDefault ();
		}
	}

	void SystemTrayHandler::Handle (const Entity& e, const NotificationRule&)
	{
		const QString& cat = e.Additional_ [AN::EF::EventCategory].toString ();
		const QString& eventId = e.Additional_ [AN::EF::EventID].toString ();

		if (cat == AN::CatEventCancel)
		{
			if (Events_.remove (eventId))
				RebuildState ();
			return;
		}

		PrepareSysTrayIcon (cat);
		PrepareLCTrayAction (cat);
		if (!Events_.contains (eventId))
		{
			EventData data;
			data.EventID_ = eventId;
			data.Count_ = 0;
			data.Category_ = cat;
			data.VisualPath_ = e.Additional_ [AN::EF::VisualPath].toStringList ();
			data.HandlingObject_ = e.Additional_ [EF::HandlingObject].value<QObject_ptr> ();
			data.Actions_ = e.Additional_ [EF::NotificationActions].toStringList ();
			data.Canceller_ = Util::MakeANCancel (e);
			Events_ [eventId] = data;
		}

		if (const int delta = e.Additional_.value (AN::EF::DeltaCount, 0).toInt ())
			Events_ [eventId].Count_ += delta;
		else
			Events_ [eventId].Count_ = e.Additional_.value (AN::EF::Count, 1).toInt ();
		Events_ [eventId].ExtendedText_ = e.Additional_ [AN::EF::ExtendedText].toString ();
		Events_ [eventId].FullText_ = e.Additional_ [AN::EF::FullText].toString ();

		// TODO migrate to co_await
		const auto& pxFuture = GetPixmap (e);
		if (pxFuture.isFinished ())
			Events_ [eventId].Pixmap_ = pxFuture.result ();
		else
			Util::Sequence (this, pxFuture) >>
					[eventId, this] (const QPixmap& px)
					{
						if (!Events_.contains (eventId))
							return;

						Events_ [eventId].Pixmap_ = px;
						RebuildState ();
					};

		RebuildState ();
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

		const auto vnv = new VisualNotificationsView { GetProxyHolder () };
		connect (vnv,
				&VisualNotificationsView::actionTriggered,
				this,
				&SystemTrayHandler::HandleActionTriggered);
		connect (vnv,
				&VisualNotificationsView::dismissEvent,
				this,
				&SystemTrayHandler::DismissNotification);
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

		const auto vnv = new VisualNotificationsView { GetProxyHolder () };
		connect (vnv,
				&VisualNotificationsView::actionTriggered,
				this,
				&SystemTrayHandler::HandleActionTriggered);
		connect (vnv,
				&VisualNotificationsView::dismissEvent,
				this,
				&SystemTrayHandler::DismissNotification);
		Action2NotificationView_ [action] = vnv;

		if (XmlSettingsManager::Instance ().property ("HideOnHoverOut").toBool ())
			new Util::UnhoverDeleteMixin (vnv, SLOT (hide ()));
	}

	void SystemTrayHandler::UpdateMenu (QMenu *menu, const QString& event, const EventData& data)
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
		// TODO Qt 5.14 remove values() call here
		auto icons2hide = Util::AsSet (Category2Icon_.values ());
		for (const auto icon : icons2hide)
			icon->contextMenu ()->clear ();

		// TODO Qt 5.14 remove values() call here
		auto actsDel = Util::AsSet (Category2Action_.values ());

		EventsForIcon_.clear ();
		EventsForAction_.clear ();

		QSet<QSystemTrayIcon*> visibleIcons;
		QSet<QAction*> actsUpd;

		for (const auto& pair : Util::Stlize (Events_))
		{
			const auto& event = pair.first;
			const auto& data = pair.second;

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

	void SystemTrayHandler::HandleActionTriggered (const QString& event, int index)
	{
		if (!Events_.contains (event))
		{
			qWarning () << Q_FUNC_INFO
					<< "no such event"
					<< event;
			return;
		}

		QMetaObject::invokeMethod (Events_ [event].HandlingObject_.get (),
				"notificationActionTriggered",
				Qt::QueuedConnection,
				Q_ARG (int, index));
	}

	void SystemTrayHandler::DismissNotification (const QString& event)
	{
		if (!Events_.contains (event))
			return;

		const auto canceller = Events_.value (event).Canceller_;
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (canceller);
	}
}
