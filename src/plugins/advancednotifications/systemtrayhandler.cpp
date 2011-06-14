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
#include <QSystemTrayIcon>
#include <QMenu>
#include "generalhandler.h"

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
				data.Category_ = cat;
				data.VisualPath_ = e.Additional_ ["org.LC.AdvNotifications.VisualPath"].toStringList ();
				data.Pixmap_ = e.Additional_ ["NotificationPixmap"].value<QPixmap> ();
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
	}
	
	void SystemTrayHandler::RebuildState ()
	{
		Q_FOREACH (QSystemTrayIcon *icon, Category2Icon_.values ())
		{
			icon->hide ();
			icon->contextMenu ()->clear ();
		}

		Q_FOREACH (const QString& event, Events_.keys ())
		{
			const EventData& data = Events_ [event];
			QSystemTrayIcon *icon = Category2Icon_ [data.Category_];
			icon->show ();

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
	}
	
	void SystemTrayHandler::handleActionTriggered ()
	{
		const QString& event = sender ()->property ("EventID").toString ();
		const int index = sender ()->property ("Index").toInt ();
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
}
}
