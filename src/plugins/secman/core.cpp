/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "core.h"
#include <interfaces/iplugin2.h>
#include <interfaces/secman/istorageplugin.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SecMan
		{
			Core::Core ()
			{
			}

			Core& Core::Instance ()
			{
				static Core c;
				return c;
			}

			bool Core::CouldHandle (const Entity& e) const
			{
				if (e.Mime_ == "x-leechcraft/data-persistent-save" ||
						e.Mime_ == "x-leechcraft/data-persistent-load")
					return true;

				return false;
			}

			void Core::Handle (Entity e)
			{
				if (e.Mime_ == "x-leechcraft/data-persistent-load")
				{
				}
				else if (e.Mime_ == "x-leechcraft/data-persistent-save")
				{
					QList<QVariant> rawKeys = e.Entity_.toList ();
					QList<QVariant> values = e.Additional_ ["Values"].toList ();

					QList<QByteArray> keys;
					Q_FOREACH (const QVariant& rKey, rawKeys)
					{
						QByteArray key = rKey.toByteArray ();
						if (key.isEmpty ())
							continue;
						keys << key;
					}

					if (values.size () != keys.size ())
					{
						qWarning () << Q_FUNC_INFO
								<< "keys size doesn't match values size, raw data:"
								<< rawKeys
								<< values;
						return;
					}

					if (keys.size () == 0)
						return;

					bool secure = e.Additional_.value ("SecureStorage", false).toBool ();
					Store (keys, values, secure);
				}
			}

			QSet<QByteArray> Core::GetExpectedPluginClasses () const
			{
				return QSet<QByteArray> () << "org.LeechCraft.SecMan.StoragePlugins/1.0";
			}

			void Core::AddPlugin (QObject *plugin)
			{
				IPlugin2 *ip2 = qobject_cast<IPlugin2*> (plugin);
				if (!ip2)
				{
					qWarning () << Q_FUNC_INFO
							<< "passed object is not a IPlugin2"
							<< plugin;
					return;
				}

				QSet<QByteArray> classes = ip2->GetPluginClasses ();
				if (classes.contains ("org.LeechCraft.SecMan.StoragePlugins/1.0"))
					AddStoragePlugin (plugin);
			}

			void Core::AddStoragePlugin (QObject *plugin)
			{
				if (!qobject_cast<IStoragePlugin*> (plugin))
				{
					qWarning () << Q_FUNC_INFO
							<< "passed object is not a IStoragePlugin"
							<< plugin;
					return;
				}
				StoragePlugins_ << plugin;
			}

			void Core::Store (const QList<QByteArray>& keys,
					const QList<QVariant>& values, bool secure)
			{
				Q_ASSERT (keys.size () == values.size ());
				Q_ASSERT (keys.size ());

				QObject *storage = GetStoragePlugin ();

				if (!storage)
				{
					qWarning () << Q_FUNC_INFO
							<< "null storage";
					return;
				}

				IStoragePlugin::StorageType type = secure ?
						IStoragePlugin::STSecure :
						IStoragePlugin::STInsecure;

				qobject_cast<IStoragePlugin*> (storage)->Save (keys, values, secure);
			}

			QObject* Core::GetStoragePlugin () const
			{
				return StoragePlugins_.size () ? StoragePlugins_.at (0) : 0;
			}
		}
	}
}
