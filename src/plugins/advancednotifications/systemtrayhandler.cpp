/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "systemtrayhandler.h"
#include <interfaces/structures.h>
#include <QMenu>
#include "generalhandler.h"

#ifdef HAVE_QML
#include "qml/visualnotificationsview.h"
#endif

namespace LeechCraft
{
namespace AdvancedNotifications
{
	SystemTrayHandler::SystemTrayHandler ()
	{
	}

	ConcreteHandlerBase::HandlerType SystemTrayHandler::GetHandlerType () const
	{
		return HTSystemTray;
	}
	
	namespace
	{
		QPixmap GetPixmap (const Entity& e, ICoreProxy_ptr proxy)
		{
			QPixmap pixmap = e.Additional_ ["NotificationPixmap"].value<QPixmap> ();
			if (pixmap.isNull ())
			{
				QString mi = "information";
				switch (e.Additional_ ["Priority"].toInt ())
				{
					case PWarning_:
						mi = "warning";
						break;
					case PCritical_:
						mi = "error";
					default:
						break;
				}
				
				pixmap = proxy->GetIcon (mi).pixmap (QSize (64, 64));
			}
			return pixmap;
		}
	}

	void SystemTrayHandler::Handle (const Entity& e)
	{
		const QString& cat = e.Additional_ ["org.LC.AdvNotifications.EventCategory"].toString ();
		const QString& eventId = e.Additional_ ["org.LC.AdvNotifications.EventID"].toString ();

		if (cat != "org.LC.AdvNotifications.Cancel")
		{
			PrepareSysTrayIcon (cat);
			if (!Events_.contains (eventId))
			{
				EventData data;
				data.EventID_ = eventId;
				data.Category_ = cat;
				data.VisualPath_ = e.Additional_ ["org.LC.AdvNotifications.VisualPath"].toStringList ();
				data.HandlingObject_ = e.Additional_ ["HandlingObject"].value<QObject_ptr> ();
				data.Actions_ = e.Additional_ ["NotificationActions"].toStringList ();
				Events_ [eventId] = data;
			}

			const int delta = e.Additional_.value ("org.LC.AdvNotifications.DeltaCount", 0).toInt ();
			if (delta)
				Events_ [eventId].Count_ += delta;
			else
				Events_ [eventId].Count_ = e.Additional_.value ("org.LC.AdvNotifications.Count", 1).toInt ();
			Events_ [eventId].ExtendedText_ = e.Additional_ ["org.LC.AdvNotifications.ExtendedText"].toString ();
			Events_ [eventId].FullText_ = e.Additional_ ["org.LC.AdvNotifications.FullText"].toString ();
			
			Events_ [eventId].Pixmap_ = GetPixmap (e, GH_->GetProxy ());
		}
		else if (!Events_.remove (eventId))
			return;
		
		RebuildState ();
	}
	
	void SystemTrayHandler::PrepareSysTrayIcon (const QString& category)
	{
		if (Category2Icon_.contains (category))
			return;
		
		QSystemTrayIcon *trayIcon = new QSystemTrayIcon (GH_->GetIconForCategory (category));
		trayIcon->setContextMenu (new QMenu ());
		Category2Icon_ [category] = trayIcon;
		
		connect (trayIcon,
				SIGNAL (activated (QSystemTrayIcon::ActivationReason)),
				this,
				SLOT (handleTrayActivated (QSystemTrayIcon::ActivationReason)));
		
#ifdef HAVE_QML
		VisualNotificationsView *vnv = new VisualNotificationsView;
		connect (vnv,
				SIGNAL (actionTriggered (const QString&, int)),
				this,
				SLOT (handleActionTriggered (const QString&, int)));
		Icon2NotificationView_ [trayIcon] = vnv;
#endif
	}
	
	void SystemTrayHandler::RebuildState ()
	{
		QSet<QSystemTrayIcon*> icons2hide;
		Q_FOREACH (QSystemTrayIcon *icon, Category2Icon_.values ())
		{
			icons2hide << icon;
			icon->contextMenu ()->clear ();
		}
		
#ifdef HAVE_QML
		EventsForIcon_.clear ();
#endif

		Q_FOREACH (const QString& event, Events_.keys ())
		{
			const EventData& data = Events_ [event];
			QSystemTrayIcon *icon = Category2Icon_ [data.Category_];
			if (!icon->isVisible ())
				icon->show ();
			icons2hide.remove (icon);
			
#ifdef HAVE_QML
			EventsForIcon_ [icon] << data;
#endif

			QMenu *menu = icon->contextMenu ();
			Q_FOREACH (const QString& pathItem, data.VisualPath_)
				menu = menu->addMenu (pathItem);
				
			if (!data.Pixmap_.isNull ())
				menu->setIcon (data.Pixmap_);
			menu->setToolTip (data.ExtendedText_);
				
			int actionIdx = 0;
			Q_FOREACH (const QString& actionName, data.Actions_)
			{
				QAction *action = menu->addAction (actionName);
				action->setProperty ("Index", actionIdx++);
				action->setProperty ("EventID", event);
				connect (action,
						SIGNAL (triggered ()),
						this,
						SLOT (handleActionTriggered ()));
			}
				
			QAction *dismiss = menu->addAction (tr ("Dismiss"));
			dismiss->setProperty ("EventID", event);
			connect (dismiss,
					SIGNAL (triggered ()),
					this,
					SLOT (dismissNotification ()));
			
			menu->addSeparator ();
			menu->addAction (data.ExtendedText_)->setEnabled (false);
		}
		
		Q_FOREACH (QSystemTrayIcon *icon, Category2Icon_.values ())
		{
			VisualNotificationsView *view = Icon2NotificationView_ [icon];
			if (!view->isVisible ())
				continue;

			const QList<EventData>& events = EventsForIcon_ [icon];
			view->SetEvents (events);
			if (events.isEmpty ())
				view->hide ();
		}
		
		Q_FOREACH (QSystemTrayIcon *icon, icons2hide)
			icon->hide ();
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
				Q_ARG (int, index));
	}
	
	void SystemTrayHandler::dismissNotification ()
	{
		const QString& event = sender ()->property ("EventID").toString ();
		if (Events_.remove (event))
			RebuildState ();
	}
	
	void SystemTrayHandler::handleTrayActivated (QSystemTrayIcon::ActivationReason reason)
	{
		if (reason != QSystemTrayIcon::Trigger)
			return;
		
#ifdef HAVE_QML
		QSystemTrayIcon *trayIcon = qobject_cast<QSystemTrayIcon*> (sender ());
		if (!trayIcon)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QSystemTrayIcon";
			return;
		}

		VisualNotificationsView *view = Icon2NotificationView_ [trayIcon];
		if (!view->isVisible ())
		{
			view->SetEvents (EventsForIcon_ [trayIcon]);
			view->move (QCursor::pos ());
		}
		view->setVisible (!view->isVisible ());
#endif
	}
}
}
