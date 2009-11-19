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

#ifndef PLUGINS_DBUSMANAGER_DBUSMANAGER_H
#define PLUGINS_DBUSMANAGER_DBUSMANAGER_H
#include <memory>
#include <QTranslator>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DBusManager
		{
			class DBusManager : public QObject
							  , public IInfo
							  , public IHaveSettings
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IHaveSettings);

				std::auto_ptr<QTranslator> Translator_;
				boost::shared_ptr<Util::XmlSettingsDialog> SettingsDialog_;
			public:
				void Init (ICoreProxy_ptr);
				void SecondInit ();
				void Release ();
				QString GetName () const;
				QString GetInfo () const;
				QStringList Provides () const;
				QStringList Uses () const;
				QStringList Needs () const;
				void SetProvider (QObject*, const QString&);
				QIcon GetIcon () const;

				boost::shared_ptr<Util::XmlSettingsDialog> GetSettingsDialog () const;
			};
		};
	};
};

#endif

