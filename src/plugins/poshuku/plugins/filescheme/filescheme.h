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

#ifndef PLUGINS_POSHUKU_PLUGINS_FILESCHEME_FILESCHEME_H
#define PLUGINS_POSHUKU_PLUGINS_FILESCHEME_FILESCHEME_H
#include <memory>
#include <QObject>
#include <QTranslator>
#include <QNetworkAccessManager>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/pluginbase.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace FileScheme
				{
					class FileScheme : public QObject
									 , public IInfo
									 , public IPlugin2
									 , public LeechCraft::Plugins::Poshuku::PluginBase
					{
						Q_OBJECT
						Q_INTERFACES (IInfo IPlugin2 LeechCraft::Plugins::Poshuku::PluginBase)

						std::auto_ptr<QTranslator> Translator_;
					public:
						void Init (ICoreProxy_ptr);
						void SecondInit ();
						void Release ();
						QString GetName () const;
						QString GetInfo () const;
						QIcon GetIcon () const;
						QStringList Provides () const;
						QStringList Needs () const;
						QStringList Uses () const;
						void SetProvider (QObject*, const QString&);

						QByteArray GetPluginClass () const;

						void Init (IProxyObject*);

						QNetworkReply* CreateRequest (IHookProxy_ptr,
								QNetworkAccessManager*,
								QNetworkAccessManager::Operation*,
								const QNetworkRequest*,
								QIODevice**);
					};
				};
			};
		};
	};
};

#endif

