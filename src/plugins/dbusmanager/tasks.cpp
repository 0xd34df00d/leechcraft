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

#include "tasks.h"
#include <QAbstractItemModel>
#include <interfaces/ijobholder.h>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DBusManager
		{
			Tasks::Tasks (QObject *parent)
			: QObject (parent)
			{
			}

			QStringList Tasks::GetHolders () const
			{
				QObjectList plugins = Core::Instance ().GetProxy ()->
					GetPluginsManager ()->GetAllCastableRoots<IJobHolder*> ();
				QStringList result;
				Q_FOREACH (QObject *plugin, plugins)
					result << qobject_cast<IInfo*> (plugin)->GetName ();
				return result;
			}

			int Tasks::RowCount (const QString& name) const
			{
				QObjectList plugins = Core::Instance ().GetProxy ()->
					GetPluginsManager ()->GetAllCastableRoots<IJobHolder*> ();
				Q_FOREACH (QObject *plugin, plugins)
				{
					if (qobject_cast<IInfo*> (plugin)->GetName () != name)
						continue;

					QAbstractItemModel *model =
						qobject_cast<IJobHolder*> (plugin)->GetRepresentation ();

					return model->rowCount ();
				}

				throw tr ("Not found job holder %1.")
					.arg (name);
			}

			QVariantList Tasks::GetData (const QString& name, int r, int role) const
			{
				QObjectList plugins = Core::Instance ().GetProxy ()->
					GetPluginsManager ()->GetAllCastableRoots<IJobHolder*> ();
				Q_FOREACH (QObject *plugin, plugins)
				{
					if (qobject_cast<IInfo*> (plugin)->GetName () != name)
						continue;

					QAbstractItemModel *model =
						qobject_cast<IJobHolder*> (plugin)->GetRepresentation ();

					QVariantList result;
					for (int i = 0, size = model->columnCount ();
							i < size; ++i)
						result << model->index (r, i).data (role);
					return result;
				}

				throw tr ("Not found job holder %1.")
					.arg (name);
			}
		};
	};
};

