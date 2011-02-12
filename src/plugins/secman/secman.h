/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Vadim Misbakh-Soloviev, Georg Rudoy
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

#ifndef PLUGINS_SECMAN_SECMAN_H
#define PLUGINS_SECMAN_SECMAN_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ipluginready.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SecMan
		{
			class Plugin : public QObject
						 , public IInfo
						 , public IEntityHandler
						 , public IPluginReady
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IEntityHandler IPluginReady)
			public:
				void Init (ICoreProxy_ptr);
				void SecondInit ();
				void Release ();
				QByteArray GetUniqueID () const;
				QString GetName () const;
				QString GetInfo () const;
				QIcon GetIcon () const;

				bool CouldHandle (const Entity&) const;
				void Handle (Entity);

				QSet<QByteArray> GetExpectedPluginClasses () const;
				void AddPlugin (QObject*);
			};
		};
	};
};

#endif

