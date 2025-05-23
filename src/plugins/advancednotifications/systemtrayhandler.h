/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMap>
#include <QStringList>
#include <util/threads/coro/taskfwd.h>
#include <interfaces/structures.h>
#include <interfaces/iactionsexporter.h>
#include "concretehandlerbase.h"
#include "eventdata.h"

class QSystemTrayIcon;

namespace LC::AdvancedNotifications
{
	class VisualNotificationsView;

	class SystemTrayHandler : public ConcreteHandlerBase
	{
		Q_OBJECT

		QMap<QString, QSystemTrayIcon*> Category2Icon_;
		QMap<QString, QAction*> Category2Action_;
		QMap<EventKey, EventData> Events_;

		QMap<QSystemTrayIcon*, QList<EventData>> EventsForIcon_;
		QMap<QSystemTrayIcon*, VisualNotificationsView*> Icon2NotificationView_;

		QMap<QAction*, QList<EventData>> EventsForAction_;
		QMap<QAction*, VisualNotificationsView*> Action2NotificationView_;
	public:
		SystemTrayHandler () = default;
		~SystemTrayHandler () override;

		NotificationMethod GetHandlerMethod () const override;
		void Handle (const Entity&, const NotificationRule&) override;
	private:
		Util::ContextTask<void> ExtractPixmap (EventData&, const Entity&);

		VisualNotificationsView* CreateVisualNotificationView ();
		void PrepareSysTrayIcon (const QString&);
		void PrepareLCTrayAction (const QString&);
		void UpdateMenu (QMenu*, const EventKey&, const EventData&);
		void RebuildState ();
		template<typename T>
		void UpdateIcon (T iconable, const QString&);
		void UpdateSysTrayIcon (QSystemTrayIcon*);
		void UpdateTrayAction (QAction*);

		void HandleActionTriggered (const EventKey&, int);
		void DismissNotification (const EventKey&);
	signals:
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace);
	};
}
