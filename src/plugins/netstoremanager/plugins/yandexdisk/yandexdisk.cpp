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

#include "yandexdisk.h"
#include <algorithm>
#include <QIcon>
#include <QSettings>
#include <QCoreApplication>
#include <util/util.h>
#include "account.h"

namespace LeechCraft
{
namespace NetStoreManager
{
namespace YandexDisk
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("netstoremanager_yandexdisk");
	}

	void Plugin::SecondInit ()
	{
		ReadAccounts ();
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.NetStoreManager.YandexDisk";
	}

	QString Plugin::GetName () const
	{
		return "NetStoreManager: Yandex.Disk";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Support for the Yandex.Disk for NetStoreManager plugin.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/netstoremanager/yandexdisk/resources/images/netstoremanager_yandexdisk.svg");
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> classes;
		classes << "org.LeechCraft.Plugins.NetStoreManager.Plugins.IStoragePlugin";
		return classes;
	}

	QObject* Plugin::GetObject ()
	{
		return this;
	}

	QString Plugin::GetStorageName () const
	{
		return tr ("Yandex.Disk");
	}

	QIcon Plugin::GetStorageIcon () const
	{
		return QIcon (":/netstoremanager/yandexdisk/resources/images/yandexnarodlogo.png");
	}

	void Plugin::RegisterAccount (const QString& accName)
	{
		Account_ptr acc (new Account (this));
		acc->SetAccountName (accName);

		if (!acc->ExecConfigDialog ())
			return;

		Accounts_ << acc;
		WriteAccounts ();
		emit accountAdded (acc.get ());
	}

	QObjectList Plugin::GetAccounts () const
	{
		QObjectList result;
		Q_FOREACH (Account_ptr acc, Accounts_)
			result << acc.get ();
		return result;
	}

	void Plugin::RemoveAccount (QObject *obj)
	{
		auto pos = std::find_if (Accounts_.begin (), Accounts_.end (),
				[obj] (decltype (Accounts_.front ()) acc)
					{ return acc.get () == obj; });
		if (pos == Accounts_.end ())
			return;

		Accounts_.erase (pos);
		WriteAccounts ();
		emit accountRemoved (obj);
	}

	void Plugin::ReadAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_NSM_YD_Accounts");
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

	void Plugin::WriteAccounts () const
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_NSM_YD_Accounts");
		settings.beginWriteArray ("Accounts");
		for (int i = 0; i < Accounts_.size (); ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue ("SerializedData", Accounts_.at (i)->Serialize ());
		}
		settings.endArray ();
	}

	void Plugin::initPlugin (QObject *proxy)
	{
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_netstoremanager_yandexdisk, LeechCraft::NetStoreManager::YandexDisk::Plugin);
