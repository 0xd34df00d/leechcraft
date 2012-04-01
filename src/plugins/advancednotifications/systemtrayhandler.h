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

#ifndef PLUGINS_ADVANCEDNOTIFICATIONS_SYSTEMTRAYHANDLER_H
#define PLUGINS_ADVANCEDNOTIFICATIONS_SYSTEMTRAYHANDLER_H
#include <functional>
#include <QMap>
#include <QStringList>
#include <QPixmap>
#include <QPointer>
#include <QSystemTrayIcon>
#include <interfaces/structures.h>
#include <interfaces/iactionsexporter.h>
#include "concretehandlerbase.h"
#include "eventdata.h"

class QSystemTrayIcon;

namespace LeechCraft
{
namespace AdvancedNotifications
{
#ifdef HAVE_QML
	class VisualNotificationsView;
#endif

	class SystemTrayHandler : public ConcreteHandlerBase
	{
		Q_OBJECT

		QMap<QString, QSystemTrayIcon*> Category2Icon_;
		QMap<QString, QAction*> Category2Action_;
		QMap<QString, EventData> Events_;

#ifdef HAVE_QML
		QMap<QSystemTrayIcon*, QList<EventData>> EventsForIcon_;
		QMap<QSystemTrayIcon*, VisualNotificationsView*> Icon2NotificationView_;

		QMap<QAction*, QList<EventData>> EventsForAction_;
		QMap<QAction*, VisualNotificationsView*> Action2NotificationView_;
#endif
	public:
		SystemTrayHandler ();

		NotificationMethod GetHandlerMethod () const;
		void Handle (const Entity&, const NotificationRule&);
	private:
		void PrepareSysTrayIcon (const QString&);
		void PrepareLCTrayAction (const QString&);
		void UpdateMenu (QMenu*, const QString&, const EventData&);
		void RebuildState ();
		template<typename T>
		void UpdateIcon (T iconable, const QString&, std::function<QSize (T)>);
		void UpdateSysTrayIcon (QSystemTrayIcon*);
		void UpdateTrayAction (QAction*);
	private slots:
		void handleActionTriggered ();
		void handleActionTriggered (const QString&, int);
		void dismissNotification ();
		void dismissNotification (const QString&);

		void handleTrayActivated (QSystemTrayIcon::ActivationReason);
		void handleLCAction ();
	signals:
		void gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace);
	};
}
}

#endif
