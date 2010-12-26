/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_ONLINEBOOKMARKS_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_ONLINEBOOKMARKS_H

#include <memory.h>
#include <QObject>
#include <QTranslator>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iplugin2.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace OnlineBookmarks
				{
					class OnlineBookmarks 	: public QObject
											, public IInfo
											, public IHaveSettings
											, public IPlugin2
					{
						Q_OBJECT
						Q_INTERFACES (IInfo IHaveSettings IPlugin2)
						
						boost::shared_ptr<Util::XmlSettingsDialog> SettingsDialog_;
						
					public:
						QIcon GetIcon () const;
						void Release ();
						void SetProvider (QObject*, const QString&);
						QStringList Uses () const;
						QStringList Needs () const;
						QStringList Provides () const;
						QString GetInfo () const;
						QString GetName () const;
						QByteArray GetUniqueID () const;
						void SecondInit ();
						void Init (ICoreProxy_ptr);
						QSet<QByteArray> GetPluginClasses () const;
						boost::shared_ptr<Util::XmlSettingsDialog> GetSettingsDialog () const;
					signals:
						void gotEntity (LeechCraft::Entity);
						void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
					};
				};
			};
		};
	};
};
#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_ONLINEBOOKMARKS_H
