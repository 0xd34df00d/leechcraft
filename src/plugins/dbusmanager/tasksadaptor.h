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

#ifndef PLUGINS_DBUSMANAGER_TASKSADAPTOR_H
#define PLUGINS_DBUSMANAGER_TASKSADAPTOR_H
#include <QDBusAbstractAdaptor>
#include <QDBusVariant>
#include <QStringList>

class QDBusMessage;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DBusManager
		{
			class Tasks;

			class TasksAdaptor : public QDBusAbstractAdaptor
			{
				Q_OBJECT

				Q_CLASSINFO ("D-Bus Interface", "org.LeechCraft.DBus.Tasks");

				Tasks *Tasks_;
			public:
				TasksAdaptor (Tasks*);
			public slots:
				QStringList GetHolders () const;
				int RowCount (const QString& holder, const QDBusMessage&) const;
				QVariantList GetData (const QString& holder,
						int row, int role,
						const QDBusMessage&) const;
			};
		};
	};
};

#endif

