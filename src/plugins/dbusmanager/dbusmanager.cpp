/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "dbusmanager.h"
#include <QIcon>
#include <interfaces/entitytesthandleresult.h>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DBusManager
		{
			void DBusManager::Init (ICoreProxy_ptr proxy)
			{
				Translator_.reset (LeechCraft::Util::InstallTranslator ("dbusmanager"));

				Core::Instance ().SetProxy (proxy);

				SettingsDialog_.reset (new Util::XmlSettingsDialog ());
				SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
						"dbusmanagersettings.xml");
			}

			void DBusManager::SecondInit ()
			{
			}

			void DBusManager::Release ()
			{
				Core::Instance ().Release ();
			}

			QByteArray DBusManager::GetUniqueID () const
			{
				return "org.LeechCraft.DBusManager";
			}

			QString DBusManager::GetName () const
			{
				return "DBus Manager";
			}

			QString DBusManager::GetInfo () const
			{
				return tr ("DBus support for LeechCraft");
			}

			QStringList DBusManager::Provides () const
			{
				return QStringList ("dbus");
			}

			QStringList DBusManager::Uses () const
			{
				return QStringList ();
			}

			QStringList DBusManager::Needs () const
			{
				return QStringList ();
			}

			void DBusManager::SetProvider (QObject*, const QString&)
			{
			}

			QIcon DBusManager::GetIcon () const
			{
				return QIcon (":/resources/images/dbusmanager.svg");
			}

			std::shared_ptr<Util::XmlSettingsDialog> DBusManager::GetSettingsDialog () const
			{
				return SettingsDialog_;
			}

			EntityTestHandleResult DBusManager::CouldHandle (const Entity& e) const
			{
				return Core::Instance ().CouldHandle (e) ?
						EntityTestHandleResult (EntityTestHandleResult::PHigh) :
						EntityTestHandleResult ();
			}

			void DBusManager::Handle (Entity e)
			{
				Core::Instance ().Handle (e);
			}
		};
	};
};

LC_EXPORT_PLUGIN (leechcraft_dbusmanager, LeechCraft::Plugins::DBusManager::DBusManager);

