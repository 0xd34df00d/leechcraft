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

#ifndef PLUGINS_DBUSMANAGER_TASKS_H
#define PLUGINS_DBUSMANAGER_TASKS_H
#include <QObject>
#include <QStringList>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DBusManager
		{
			class Tasks : public QObject
			{
				Q_OBJECT
			public:
				Tasks (QObject* = 0);

				QStringList GetHolders () const;
				int RowCount (const QString& holder) const;
				int ColumnCount (const QString& holder) const;
				QVariant GetData (const QString&, int, int, int) const;
			};
		};
	};
};

#endif

