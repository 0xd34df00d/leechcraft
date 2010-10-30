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

#ifndef PLUGINS_SECMAN_CORE_H
#define PLUGINS_SECMAN_CORE_H
#include <QObject>
#include <interfaces/structures.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SecMan
		{
			class Core : public QObject
			{
				Core ();

				QObjectList StoragePlugins_;
			public:
				static Core& Instance ();

				bool CouldHandle (const Entity&) const;
				void Handle (Entity);
				QSet<QByteArray> GetExpectedPluginClasses () const;
				void AddPlugin (QObject*);
			private:
				/** This one is called internally from AddPlugin, so it
				 * has no need to make sanity checks of the object.
				 *
				 * @param[in] object The storage plugin instance object.
				 */
				void AddStoragePlugin (QObject *object);
				void Store (const QList<QByteArray>&, const QList<QVariant>&, bool);
				QList<QVariant> Load (const QList<QByteArray>&, bool);

				QObject* GetStoragePlugin () const;
			};
		}
	}
}

#endif
