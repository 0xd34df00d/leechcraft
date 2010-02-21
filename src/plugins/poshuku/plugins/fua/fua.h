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

#ifndef PLUGINS_POSHUKU_PLUGINS_FUA_FUA_H
#define PLUGINS_POSHUKU_PLUGINS_FUA_FUA_H
#include <memory>
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <QMap>
#include <QTranslator>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iplugin2.h>
#include <interfaces/pluginbase.h>

class QStandardItemModel;

namespace LeechCraft
{
	namespace Util
	{
		class XmlSettingsDialog;
	};
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace Fua
				{
					class Settings;

					class FUA : public QObject
							  , public IInfo
							  , public IPlugin2
							  , public IHaveSettings
							  , public LeechCraft::Plugins::Poshuku::PluginBase
					{
						Q_OBJECT
						Q_INTERFACES (IInfo IPlugin2 IHaveSettings LeechCraft::Plugins::Poshuku::PluginBase)

						boost::shared_ptr<QStandardItemModel> Model_;
						boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> XmlSettingsDialog_;
						std::auto_ptr<QTranslator> Translator_;
						QMap<QString, QString> Browser2ID_;
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

						QSet<QByteArray> GetPluginClasses () const;

						boost::shared_ptr<Util::XmlSettingsDialog> GetSettingsDialog () const;

						void Init (IProxyObject*);
						QString OnUserAgentForUrl (const QWebPage*, const QUrl&);

						void Save () const;
						const QMap<QString, QString>& GetBrowser2ID () const;
					};
				};
			};
		};
	};
};

#endif

