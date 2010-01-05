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

#include "general.h"
#include <QBuffer>
#include <QPixmap>
#include <QIcon>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DBusManager
		{
			General::General (QObject *parent)
			: QObject (parent)
			{
			}

			QStringList General::GetLoadedPlugins ()
			{
				QObjectList plugins = Core::Instance ().GetProxy ()->
					GetPluginsManager ()->GetAllPlugins ();
				QStringList result;
				Q_FOREACH (QObject *plugin, plugins)
					result << qobject_cast<IInfo*> (plugin)->GetName ();

				return result;
			}

			QString General::GetDescription (const QString& name)
			{
				QObjectList plugins = Core::Instance ().GetProxy ()->
					GetPluginsManager ()->GetAllPlugins ();
				Q_FOREACH (QObject *plugin, plugins)
				{
					IInfo *ii = qobject_cast<IInfo*> (plugin);
					if (ii->GetName () == name)
						return ii->GetInfo ();
				}

				throw tr ("Not found plugin %1.")
					.arg (name);
			}

			QByteArray General::GetIcon (const QString& name, int dim)
			{
				QObjectList plugins = Core::Instance ().GetProxy ()->
					GetPluginsManager ()->GetAllPlugins ();
				Q_FOREACH (QObject *plugin, plugins)
				{
					IInfo *ii = qobject_cast<IInfo*> (plugin);
					if (ii->GetName () != name)
						continue;

					QIcon icon = ii->GetIcon ();
					QPixmap pixmap = icon.pixmap (dim, dim);
					QBuffer buffer;
					if (!pixmap.save (&buffer, "PNG", 100))
						throw tr ("Could not save icon for plugin %1 to PNG %2x%2")
							.arg (name)
							.arg (dim);
					return buffer.data ();
				}
				
				throw tr ("Not found plugin %1.")
					.arg (name);
			}
		};
	};
};

