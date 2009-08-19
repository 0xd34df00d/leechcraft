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

#ifndef PLUGINS_DBUSMANAGER_ADAPTOR_H
#define PLUGINS_DBUSMANAGER_ADAPTOR_H
#include <QDBusAbstractAdaptor>
#include <QStringList>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DBusManager
		{
			class Core;
			class QDBusMessage;

			class Adaptor : public QDBusAbstractAdaptor
			{
				Q_OBJECT

				Q_CLASSINFO ("D-Bus Interface", "org.LeechCraft.DBus.Manager");
				Q_PROPERTY (QString OrganizationName READ GetOrganizationName);
				Q_PROPERTY (QString ApplicationName READ GetApplicationName);

				Core *Core_;
			public:
				Adaptor (Core*);

				QString GetOrganizationName () const;
				QString GetApplicationName () const;
			public slots:
				QString Greeter (const QString&, const QDBusMessage&);
				QStringList GetLoadedPlugins ();
			signals:
				void aboutToQuit ();
				void someEventHappened (const QString&);
			};
		};
	};
};

#endif

