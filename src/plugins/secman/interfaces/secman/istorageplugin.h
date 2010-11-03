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

#ifndef PLUGINS_SECMAN_INTERFACES_ISTORAGEPLUGIN_H
#define PLUGINS_SECMAN_INTERFACES_ISTORAGEPLUGIN_H
#include <QtPlugin>
#include <QFlags>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SecMan
		{
			class IStoragePlugin
			{
			public:
				virtual ~IStoragePlugin () {}

				enum StorageType
				{
					STInsecure,
					STSecure
				};

				Q_DECLARE_FLAGS (StorageTypes, StorageType);

				virtual StorageTypes GetStorageTypes () const = 0;
				virtual QList<QByteArray> ListKeys (StorageType st = STInsecure) = 0;
				virtual void Save (const QByteArray& key, const QVariantList& value, StorageType st = STInsecure) = 0;
				virtual QVariantList Load (const QByteArray& key, StorageType st = STInsecure) = 0;
				virtual void Save (const QList<QPair<QByteArray, QVariantList> >& keyValues, StorageType st = STInsecure) = 0;
				virtual QList<QVariantList> Load (const QList<QByteArray>& keys, StorageType st = STInsecure) = 0;
			};
		}
	}
}

Q_DECLARE_INTERFACE (LeechCraft::Plugins::SecMan::IStoragePlugin,
		"org.Deviant.LeechCraft.Plugins.SecMan.IStoragePlugin/1.0");

#endif
