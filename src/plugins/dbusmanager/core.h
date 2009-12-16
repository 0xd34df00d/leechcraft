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

#ifndef PLUGINS_DBUSMANAGER_CORE_H
#define PLUGINS_DBUSMANAGER_CORE_H
#include <memory>
#include <QObject>
#include <QDBusConnection>
#include <QStringList>
#include <interfaces/iinfo.h>
#include "notificationmanager.h"
#include "general.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DBusManager
		{
			class Core : public QObject
			{
				Q_OBJECT

				std::auto_ptr<QDBusConnection> Connection_;
				std::auto_ptr<NotificationManager> NotificationManager_;
				std::auto_ptr<General> General_;

				ICoreProxy_ptr Proxy_;

				Core ();
			public:
				static Core& Instance ();
				void Release ();
				void SetProxy (ICoreProxy_ptr);
				ICoreProxy_ptr GetProxy () const;
				QString Greeter (const QString&);
			private:
				void DumpError ();
			private slots:
				void doDelayedInit ();
			};
		};
	};
};

#endif

