/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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

#ifndef PLUGINS_KINOTIFY_KINOTIFY_H
#define PLUGINS_KINOTIFY_KINOTIFY_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ihavesettings.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>

namespace LeechCraft
{
	namespace Util
	{
		class ResourceLoader;
	}

	namespace Plugins
	{
		namespace Kinotify
		{

			class KinotifyWidget;

			class Plugin : public QObject
						 , public IInfo
						 , public IEntityHandler
						 , public IHaveSettings
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IEntityHandler IHaveSettings)

				ICoreProxy_ptr Proxy_;
				QList<KinotifyWidget*> ActiveNotifications_;
				Util::XmlSettingsDialog_ptr SettingsDialog_;
				boost::shared_ptr<Util::ResourceLoader> ThemeLoader_;
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

				Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
			public slots:
				void pushNotification ();
			};
		};
	};
};

#endif

