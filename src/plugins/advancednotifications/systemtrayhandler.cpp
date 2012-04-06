/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
#include <interfaces/core/icoreproxy.h>
#include <QMenu>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include "generalhandler.h"
#include "xmlsettingsmanager.h"

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

	NotificationMethod SystemTrayHandler::GetHandlerMethod () const
	{
		return NMTray;
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

				pixmap = proxy->GetIcon ("dialog-" + mi).pixmap (QSize (64, 64));
			}
			return pixmap;
		}
	}

	void SystemTrayHandler::Handle (const Entity& e, const NotificationRule&)
	{
		const QString& cat = e.Additional_ ["org.LC.AdvNotifications.EventCategory"].toString ();
		const QString& eventId = e.Additional_ ["org.LC.AdvNotifications.EventID"].toString ();

		if (cat != "org.LC.AdvNotifications.Cancel")
		{
			PrepareSysTrayIcon (cat);
			PrepareLCTrayAction (cat);
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
		connect (vnv,
				SIGNAL (dismissEvent (const QString&)),
				this,
				SLOT (dismissNotification (const QString&)));
		Icon2NotificationView_ [trayIcon] = vnv;
#endif
	}

	void SystemTrayHandler::PrepareLCTrayAction (const QString& category)
	{
		if (Category2Action_.contains (category))
			return;

		QAction *action = new QAction (GH_->GetIconForCategory (category), category, this);
		action->setMenu (new QMenu ());
		Category2Action_ [category] = action;

		connect (action,
				SIGNAL (triggered ()),
				this,
				SLOT (handleLCAction ()));

		emit gotActions (QList<QAction*> () << action, AEPLCTray);

#ifdef HAVE_QML
		VisualNotificationsView *vnv = new VisualNotificationsView;
		connect (vnv,
				SIGNAL (actionTriggered (const QString&, int)),
				this,
				SLOT (handleActionTriggered (const QString&, int)));
		connect (vnv,
				SIGNAL (dismissEvent (const QString&)),
				this,
				SLOT (dismissNotification (const QString&)));
		Action2NotificationView_ [action] = vnv;
#endif
	}

	void SystemTrayHandler::UpdateMenu (QMenu *menu, const QString& event, const EventData& data)
	{
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
		auto icons2hide = Category2Icon_.values ().toSet ();
		Q_FOREACH (QSystemTrayIcon *icon, icons2hide)
			icon->contextMenu ()->clear ();

		auto actsDel = Category2Action_.values ().toSet ();
		Q_FOREACH (QAction *action, actsDel)
			action->menu ()->clear ();

#ifdef HAVE_QML
		EventsForIcon_.clear ();
		EventsForAction_.clear ();
#endif

		QSet<QSystemTrayIcon*> visibleIcons;
		QSet<QAction*> actsUpd;

		Q_FOREACH (const QString& event, Events_.keys ())
		{
			const EventData& data = Events_ [event];

			QSystemTrayIcon *icon = Category2Icon_ [data.Category_];
			icons2hide.remove (icon);
			visibleIcons << icon;

			QAction *action = Category2Action_ [data.Category_];
			actsDel.remove (action);
			actsUpd << action;

#ifdef HAVE_QML
			EventsForIcon_ [icon] << data;
			EventsForAction_ [action] << data;
#endif

			UpdateMenu (icon->contextMenu (), event, data);
			UpdateMenu (action->menu (), event, data);
		}

#ifdef HAVE_QML
		Q_FOREACH (QSystemTrayIcon *icon, Category2Icon_.values ())
		{
			VisualNotificationsView *view = Icon2NotificationView_ [icon];
			if (!view->isVisible ())
				continue;

			const auto& events = EventsForIcon_ [icon];
			view->SetEvents (events);
			if (events.isEmpty ())
				view->hide ();
		}

		Q_FOREACH (QAction *action, Category2Action_.values ())
		{
			VisualNotificationsView *view = Action2NotificationView_ [action];
			if (!view->isVisible ())
				continue;

			const auto& events = EventsForAction_ [action];
			view->SetEvents (events);
			if (events.isEmpty ())
				view->hide ();
		}
#endif

		Q_FOREACH (QSystemTrayIcon *icon, visibleIcons)
		{
			if (!icon->isVisible ())
				icon->show ();
			UpdateSysTrayIcon (icon);
		}

		Q_FOREACH (QAction *action, actsUpd)
			UpdateTrayAction (action);

		Q_FOREACH (QSystemTrayIcon *icon, icons2hide)
			icon->hide ();

		Q_FOREACH (QAction *action, actsDel)
			Category2Action_.remove (Category2Action_.key (action));
		qDeleteAll (actsDel);
	}

	namespace
	{
		void FitSize (QFont& font, const QSize& iconSize, const QString& countText,
				std::function<int (QFont)> g, std::function<void (QFont&, int)> s)
		{
			s (font, ((iconSize.height () + 2 * g (font)) / 3));
			int numIters = 0;
			while (true)
			{
				const int width = QFontMetrics (font).width (countText);
				if (width > iconSize.width () ||
						g (font) >= iconSize.height ())
					s (font, g (font) - 1);
				else
					break;

				if (++numIters >= 12)
					break;
			}
		}
	}

	template<typename T>
	void SystemTrayHandler::UpdateIcon (T iconable, const QString& category, std::function<QSize (T)> iconSizeGetter)
	{
		QIcon icon = GH_->GetIconForCategory (category);
		if (!XmlSettingsManager::Instance ()
				.property ("EnableCounter." + category.toLatin1 ()).toBool ())
		{
			iconable->setIcon (icon);
			return;
		}

		const QSize& iconSize = iconSizeGetter (iconable);
		QPixmap px = icon.pixmap (iconSize);

		int eventCount = 0;
		Q_FOREACH (const EventData& event, Events_.values ())
			if (event.Category_ == category)
				eventCount += event.Count_;

		const QString& countText = QString::number (eventCount);

		QFont font = qApp->font ();
		font.setBold (true);
		font.setItalic (true);

		// Cause gcc 4.5.x sucks and fails to compile without being such explicit.
		std::function<int (QFont)> getPointSize = [] (QFont f) { return f.pointSize (); };
		std::function<int (QFont)> getPixelSize = [] (QFont f) { return f.pixelSize (); };
		auto gFunc = font.pointSize () > 1 ? getPointSize : getPixelSize;

		std::function<void (QFont&, int)> setPointSize = [] (QFont& f, int size) { f.setPointSize (size); };
		std::function<void (QFont&, int)> setPixelSize = [] (QFont& f, int size) { f.setPixelSize (size); };
		auto sFunc = font.pointSize () > 1 ? setPointSize : setPixelSize;
		FitSize (font, iconSize, countText, gFunc, sFunc);

		const bool tooSmall = gFunc (font) < 5;
		if (tooSmall)
			sFunc (font, gFunc (qApp->font ()));

		QPainter p (&px);
		p.setFont (font);
		p.setPen (Qt::darkCyan);
		p.drawText (0, 1,
				iconSize.width (), iconSize.height (),
				Qt::AlignBottom | Qt::AlignRight,
				tooSmall ? "#" : countText);
		p.end ();

		iconable->setIcon (QIcon (px));
	}

	void SystemTrayHandler::UpdateSysTrayIcon (QSystemTrayIcon *trayIcon)
	{
		const QString& category = Category2Icon_.key (trayIcon);
		UpdateIcon<QSystemTrayIcon*> (trayIcon, category,
				[] (QSystemTrayIcon *icon) { return icon->geometry ().size (); });
	}

	void SystemTrayHandler::UpdateTrayAction (QAction *action)
	{
		const QString& category = Category2Action_.key (action);
		UpdateIcon<QAction*> (action, category,
				[] (QAction*) { return QSize (22, 22); });
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
		dismissNotification (sender ()->property ("EventID").toString ());
	}

	void SystemTrayHandler::dismissNotification (const QString& event)
	{
		if (Events_.remove (event))
			RebuildState ();
	}

	namespace
	{
		void ShowVNV (VisualNotificationsView *view, const QList<EventData>& events)
		{
			if (!view->isVisible ())
			{
				view->SetEvents (events);

				QPoint pos = QCursor::pos ();
				const QRect& geometry = qApp->desktop ()->screenGeometry (pos);
				const QSize& size = view->size ();
				const bool dropDown = pos.y () < geometry.height () / 2;
				const bool dropRight = pos.x () + size.width () < geometry.width ();

				if (!dropDown)
					pos.ry () -= size.height ();
				if (!dropRight)
					pos.rx () -= size.width ();

				view->move (pos);
			}
			view->setVisible (!view->isVisible ());
		}
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

		ShowVNV (Icon2NotificationView_ [trayIcon], EventsForIcon_ [trayIcon]);
#endif
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

#ifndef HAVE_QML
		action->menu ()->show ();
#else
		ShowVNV (Action2NotificationView_ [action], EventsForAction_ [action]);
#endif
	}
}
}
