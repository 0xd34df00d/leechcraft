/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "systemtrayhandler.h"
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ientitymanager.h>
#include <QMenu>
#include <QPainter>
#include <QApplication>
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
#include "fields.h"

namespace LC
{
namespace AdvancedNotifications
{
	SystemTrayHandler::SystemTrayHandler ()
	{
	}

	SystemTrayHandler::~SystemTrayHandler ()
	{
		const auto& icons = Category2Icon_.values ();
		for (auto icon : icons)
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
			const auto& pxVar = e.Additional_ ["NotificationPixmap"];

			if (pxVar.canConvert<QPixmap> ())
				return Util::MakeReadyFuture (pxVar.value<QPixmap> ());

			if (pxVar.canConvert<QImage> ())
				return Util::MakeReadyFuture (QPixmap::fromImage (pxVar.value<QImage> ()));

			const auto prio = e.Additional_ ["Priority"].value<Priority> ();
			auto getDefault = [prio]
			{
				QString mi = "information";
				switch (prio)
				{
				case Priority::Warning:
					mi = "warning";
					break;
				case Priority::Critical:
					mi = "error";
				default:
					break;
				}

				const auto& pixmap = GetProxyHolder ()->GetIconThemeManager ()->
						GetIcon ("dialog-" + mi).pixmap (QSize (64, 64));
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
		const QString& cat = e.Additional_ [Fields::EventCategory].toString ();
		const QString& eventId = e.Additional_ [Fields::EventID].toString ();

		if (cat == Fields::Values::CancelEvent)
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
			data.VisualPath_ = e.Additional_ ["org.LC.AdvNotifications.VisualPath"].toStringList ();
			data.HandlingObject_ = e.Additional_ ["HandlingObject"].value<QObject_ptr> ();
			data.Actions_ = e.Additional_ ["NotificationActions"].toStringList ();
			data.Canceller_ = Util::MakeANCancel (e);
			Events_ [eventId] = data;
		}

		if (const int delta = e.Additional_.value ("org.LC.AdvNotifications.DeltaCount", 0).toInt ())
			Events_ [eventId].Count_ += delta;
		else
			Events_ [eventId].Count_ = e.Additional_.value ("org.LC.AdvNotifications.Count", 1).toInt ();
		Events_ [eventId].ExtendedText_ = e.Additional_ ["org.LC.AdvNotifications.ExtendedText"].toString ();
		Events_ [eventId].FullText_ = e.Additional_ ["org.LC.AdvNotifications.FullText"].toString ();

		const auto& pxFuture = GetPixmap (e);
		if (pxFuture.isFinished ())
			Events_ [eventId].Pixmap_ = pxFuture;
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

	void SystemTrayHandler::PrepareSysTrayIcon (const QString& category)
	{
#ifdef Q_OS_MAC
		return;
#endif

		if (Category2Icon_.contains (category))
			return;

		QSystemTrayIcon *trayIcon = new QSystemTrayIcon (GH_->GetIconForCategory (category));
		trayIcon->setContextMenu (new QMenu ());
		Category2Icon_ [category] = trayIcon;

		connect (trayIcon,
				SIGNAL (activated (QSystemTrayIcon::ActivationReason)),
				this,
				SLOT (handleTrayActivated (QSystemTrayIcon::ActivationReason)));

		const auto vnv = new VisualNotificationsView { GetProxyHolder () };
		connect (vnv,
				SIGNAL (actionTriggered (const QString&, int)),
				this,
				SLOT (handleActionTriggered (const QString&, int)));
		connect (vnv,
				SIGNAL (dismissEvent (const QString&)),
				this,
				SLOT (dismissNotification (const QString&)));
		Icon2NotificationView_ [trayIcon] = vnv;

		if (XmlSettingsManager::Instance ().property ("HideOnHoverOut").toBool ())
			new Util::UnhoverDeleteMixin (vnv, SLOT (hide ()));
	}

	void SystemTrayHandler::PrepareLCTrayAction (const QString& category)
	{
		if (Category2Action_.contains (category))
			return;

		const auto action = new QAction (GH_->GetIconForCategory (category), category, this);
		Category2Action_ [category] = action;

		connect (action,
				SIGNAL (triggered ()),
				this,
				SLOT (handleLCAction ()));

		emit gotActions ({ action }, ActionsEmbedPlace::LCTray);

		const auto vnv = new VisualNotificationsView { GetProxyHolder () };
		connect (vnv,
				SIGNAL (actionTriggered (const QString&, int)),
				this,
				SLOT (handleActionTriggered (const QString&, int)));
		connect (vnv,
				SIGNAL (dismissEvent (const QString&)),
				this,
				SLOT (dismissNotification (const QString&)));
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
			QAction *action = menu->addAction (actionName);
			action->setProperty ("Index", actionIdx++);
			action->setProperty ("EventID", event);
			connect (action,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionTriggered ()), Qt::QueuedConnection);
		}

		QAction *dismiss = menu->addAction (tr ("Dismiss"));
		dismiss->setProperty ("EventID", event);
		connect (dismiss,
				SIGNAL (triggered ()),
				this,
				SLOT (dismissNotification ()), Qt::QueuedConnection);

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
		QIcon icon = GH_->GetIconForCategory (category);
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

		const auto& palette = qApp->palette ();

		QFont font = qApp->font ();
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

	void SystemTrayHandler::handleActionTriggered ()
	{
		const QString& event = sender ()->property ("EventID").toString ();
		const int index = sender ()->property ("Index").toInt ();

		handleActionTriggered (event, index);
	}

	void SystemTrayHandler::handleActionTriggered (const QString& event, int index)
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

	void SystemTrayHandler::dismissNotification ()
	{
		dismissNotification (sender ()->property ("EventID").toString ());
	}

	void SystemTrayHandler::dismissNotification (const QString& event)
	{
		if (!Events_.contains (event))
			return;

		const auto canceller = Events_.value (event).Canceller_;
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (canceller);
	}

	namespace
	{
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

	void SystemTrayHandler::handleTrayActivated (QSystemTrayIcon::ActivationReason reason)
	{
		if (reason != QSystemTrayIcon::Trigger)
			return;

		QSystemTrayIcon *trayIcon = qobject_cast<QSystemTrayIcon*> (sender ());
		if (!trayIcon)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QSystemTrayIcon";
			return;
		}

		ShowVNV (Icon2NotificationView_ [trayIcon], EventsForIcon_ [trayIcon]);
	}

	void SystemTrayHandler::handleLCAction ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QSystemTrayIcon";
			return;
		}

		ShowVNV (Action2NotificationView_ [action], EventsForAction_ [action]);
	}
}
}
