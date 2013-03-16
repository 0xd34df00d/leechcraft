/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "googledrive.h"
#include <QIcon>
#include <QSettings>
#include <QCoreApplication>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include "authmanager.h"
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace NetStoreManager
{
namespace GoogleDrive
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("netstoremanager_googledrive");

		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog);
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"nsmgoogledrivesettings.xml");

		Core::Instance ().SetProxy (proxy);
		AuthManager_ = new AuthManager (this);

		connect (AuthManager_,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
		connect (&Core::Instance (),
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
		connect (&Core::Instance (),
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)));

		connect (AuthManager_,
				SIGNAL (authSuccess (QObject*)),
				this,
				SLOT (handleAuthSuccess (QObject*)));

		ReadAccounts ();
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.NetStoreManager.GoogleDrive";
	}

	QString Plugin::GetName () const
	{
		return "NetStoreManager: GoogleDrive";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Support for the GoogleDrive for NetStoreManager plugin.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon (":/netstoremanager/googledrive/resources/images/googledrive.svg");
		return icon;
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> classes;
		classes << "org.LeechCraft.Plugins.NetStoreManager.Plugins.IStoragePlugin";
		return classes;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	QObject* Plugin::GetQObject ()
	{
		return this;
	}

	QObjectList Plugin::GetAccounts () const
	{
		QObjectList result;
		std::transform (Accounts_.begin (), Accounts_.end (),
				std::back_inserter (result),
				[] (decltype (Accounts_.front ()) acc) { return acc.get (); });

		return result;
	}

	QIcon Plugin::GetStorageIcon () const
	{
		return QIcon (":/netstoremanager/googledrive/resources/images/googledrivelogo.png");
	}

	QString Plugin::GetStorageName () const
	{
		return "Google Drive";
	}

	void Plugin::RegisterAccount (const QString& name)
	{
		Account *account = new Account (name, this);
		AuthManager_->Auth (account);
	}

	void Plugin::RemoveAccount (QObject *accObj)
	{
		auto pos = std::find_if (Accounts_.begin (), Accounts_.end (),
				[accObj] (decltype (Accounts_.front ()) acc)
					{ return acc.get () == accObj; });
		if (pos == Accounts_.end ())
			return;

		Accounts_.erase (pos);
		WriteAccounts ();
		emit accountRemoved (accObj);
	}

	void Plugin::WriteAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_NSM_GD_Accounts");
		settings.beginWriteArray ("Accounts");
		for (int i = 0; i < Accounts_.size (); ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue ("SerializedData", Accounts_.at (i)->Serialize ());
		}
		settings.endArray ();
	}

	void Plugin::ReadAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_NSM_GD_Accounts");
		int size = settings.beginReadArray ("Accounts");
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			const QByteArray& data = settings.value ("SerializedData").toByteArray ();
			Account_ptr acc = Account::Deserialize (data, this);
			Accounts_ << acc;
			emit accountAdded (acc.get ());
		}
		settings.endArray ();
	}

	void Plugin::handleAuthSuccess (QObject *accObj)
	{
		Account_ptr acc (qobject_cast<Account*> (accObj));
		Accounts_ << acc;
		WriteAccounts ();
		emit accountAdded (accObj);
	}

}
}
}


LC_EXPORT_PLUGIN (leechcraft_netstoremanager_googeldrive, LeechCraft::NetStoreManager::GoogleDrive::Plugin);
