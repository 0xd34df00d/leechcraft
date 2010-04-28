/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGINS_DBUSMANAGER_NOTIFICATIONMANAGER_H
#define PLUGINS_DBUSMANAGER_NOTIFICATIONMANAGER_H
#include <memory>
#include <QObject>
#include <QDBusInterface>
#include <interfaces/iinfo.h>

class QDBusPendingCallWatcher;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DBusManager
		{
			class NotificationManager : public QObject
			{
				Q_OBJECT

				std::auto_ptr<QDBusInterface> Connection_;
				struct ActionData
				{
					QObject *Handler_;
					QStringList Actions_;
				};
				QMap<QDBusPendingCallWatcher*, ActionData> Watcher2AD_;
				QMap<uint, ActionData> CallID2AD_;
			public:
				NotificationManager (QObject* = 0);

				void Init ();
				bool CouldNotify (const DownloadEntity&) const;
				void HandleNotification (const DownloadEntity&);
			private slots:
				void handleNotificationCallFinished (QDBusPendingCallWatcher*);
				void handleActionInvoked (uint, QString);
			};
		};
	};
};

#endif

