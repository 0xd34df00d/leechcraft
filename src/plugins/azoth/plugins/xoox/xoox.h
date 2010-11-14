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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_XOOX_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_XOOX_H
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/iprotocolplugin.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			namespace Plugins
			{
				namespace Xoox
				{
					class GlooxProtocol;

					class Plugin : public QObject
								 , public IInfo
								 , public IPlugin2
								 , public IProtocolPlugin
					{
						Q_OBJECT
						Q_INTERFACES (IInfo IPlugin2 LeechCraft::Plugins::Azoth::Plugins::IProtocolPlugin)

						boost::shared_ptr<GlooxProtocol> GlooxProtocol_;
					public:
						void Init (ICoreProxy_ptr);
						void SecondInit ();
						void Release ();
						QByteArray GetUniqueID () const;
						QString GetName () const;
						QString GetInfo () const;
						QIcon GetIcon () const;
						QStringList Provides () const;
						QStringList Needs () const;
						QStringList Uses () const;
						void SetProvider (QObject*, const QString&);

						QSet<QByteArray> GetPluginClasses () const;

						QObject* GetObject ();
						QList<IProtocol*> GetProtocols () const;
					signals:
						void gotEntity (const LeechCraft::Entity&);
					};
				}
			}
		}
	}
}

#endif

