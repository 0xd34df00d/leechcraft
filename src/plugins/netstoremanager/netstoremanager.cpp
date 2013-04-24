/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "netstoremanager.h"
#include <QIcon>
#include <interfaces/core/ientitymanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include "interfaces/netstoremanager/istorageplugin.h"
#include "interfaces/netstoremanager/istorageaccount.h"
#include "managertab.h"
#include "xmlsettingsmanager.h"
#include "accountsmanager.h"
#include "accountslistwidget.h"
#include "upmanager.h"
#include "syncmanager.h"
#include "syncwidget.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("netstoremanager");

		ManagerTC_ =
		{
			GetUniqueID () + ".manager",
			"NetStoreManager",
			GetInfo (),
			GetIcon (),
			45,
			TFOpenableByRequest
		};

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "netstoremanagersettings.xml");

		AccountsManager_ = new AccountsManager (this);
		XSD_->SetCustomWidget ("AccountsWidget", new AccountsListWidget (AccountsManager_));

		UpManager_ = new UpManager (proxy, this);

		connect (UpManager_,
				SIGNAL (fileUploaded (QString, QUrl)),
				this,
				SIGNAL (fileUploaded (QString, QUrl)));

		Proxy_ = proxy;
	}

	void Plugin::SecondInit ()
	{
		SyncManager_ = new SyncManager (AccountsManager_, this);
		SyncWidget *w = new SyncWidget (AccountsManager_);
		w->RestoreData ();
		XSD_->SetCustomWidget ("SyncWidget", w);
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.NetStoreManager";
	}

	void Plugin::Release ()
	{
		SyncManager_->Release ();
	}

	QString Plugin::GetName () const
	{
		return "NetStoreManager";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Allows one to manage his network storages like Yandex.Disk.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon (":/netstoremanager/resources/images/netstoremanager.svg");
		return icon;
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return TabClasses_t () << ManagerTC_;
	}

	void Plugin::TabOpenRequested (const QByteArray& id)
	{
		if (id == ManagerTC_.TabClass_)
		{
			ManagerTab *tab = new ManagerTab (ManagerTC_, AccountsManager_,
					Proxy_, this);
			emit addNewTab (tr ("Net storage"), tab);
			emit changeTabIcon (tab, GetIcon ());
			emit raiseTab (tab);
			connect (tab,
					SIGNAL (removeTab (QWidget*)),
					this,
					SIGNAL (removeTab (QWidget*)));
			connect (tab,
					SIGNAL (uploadRequested (IStorageAccount*, QString, QByteArray, bool)),
					UpManager_,
					SLOT (handleUploadRequest (IStorageAccount*, QString, QByteArray, bool)));
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown ID"
					<< id;
	}

	QSet<QByteArray> Plugin::GetExpectedPluginClasses () const
	{
		QSet<QByteArray> classes;
		classes << "org.LeechCraft.Plugins.NetStoreManager.Plugins.IStoragePlugin";
		return classes;
	}

	void Plugin::AddPlugin (QObject *pluginObj)
	{
		IStoragePlugin *plugin = qobject_cast<IStoragePlugin*> (pluginObj);
		if (plugin)
			AccountsManager_->AddPlugin (plugin);
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	QAbstractItemModel* Plugin::GetRepresentation () const
	{
		return UpManager_->GetRepresentationModel ();
	}

	QStringList Plugin::GetServiceVariants () const
	{
		QStringList result;
		Q_FOREACH (auto account, AccountsManager_->GetAccounts ())
		{
			auto parent = qobject_cast<IStoragePlugin*> (account->GetParentPlugin ());
			result << QString ("%1: %2")
					.arg (parent->GetStorageName ())
					.arg (account->GetAccountName ());
		}
		return result;
	}

	void Plugin::UploadFile (const QString& filename, const QString& service)
	{
		const int idx = GetServiceVariants ().indexOf (service);
		auto account = AccountsManager_->GetAccounts ().value (idx);
		if (!account)
		{
			Proxy_->GetEntityManager ()->HandleEntity (Util::MakeNotification ("NetStoreManager",
					tr ("No account for service name %1.")
						.arg ("<em>" + service + "</em>"),
					PCritical_));
			return;
		}

		UpManager_->handleUploadRequest (account, filename);
		UpManager_->ScheduleAutoshare (filename);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_netstoremanager, LeechCraft::NetStoreManager::Plugin);
