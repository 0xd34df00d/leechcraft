/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#ifndef PLUGINS_SECMAN_PLUGINS_SIMPLESTORAGE_SIMPLESTORAGE_H
#define PLUGINS_SECMAN_PLUGINS_SIMPLESTORAGE_SIMPLESTORAGE_H
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/secman/istorageplugin.h>

class QSettings;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SecMan
		{
			namespace StoragePlugins
			{
				namespace SimpleStorage
				{
					class Plugin : public QObject
								 , public IInfo
								 , public IPlugin2
								 , public IStoragePlugin
					{
						Q_OBJECT
						Q_INTERFACES (IInfo IPlugin2 LeechCraft::Plugins::SecMan::IStoragePlugin)

						boost::shared_ptr<QSettings> Storage_;
					public:
						void Init (ICoreProxy_ptr);
						void SecondInit ();
						QByteArray GetUniqueID () const;
						void Release ();
						QString GetName () const;
						QString GetInfo () const;
						QIcon GetIcon () const;
						QStringList Provides () const;
						QStringList Needs () const;
						QStringList Uses () const;
						void SetProvider (QObject*, const QString&);

						QSet<QByteArray> GetPluginClasses () const;

						StorageTypes GetStorageTypes () const;
						QList<QByteArray> ListKeys (StorageType);
						void Save (const QByteArray&, const QVariant&, StorageType);
						QVariant Load (const QByteArray&, StorageType);
						void Save (const QList<QPair<QByteArray, QVariant> >&, StorageType);
						QList<QVariant> Load (const QList<QByteArray>&, StorageType);
					};
				}
			}
		}
	}
}

#endif

