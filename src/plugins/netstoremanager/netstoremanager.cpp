/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "netstoremanager.h"
#include <QIcon>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ientitymanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include <util/xpc/util.h>
#include "interfaces/netstoremanager/istorageplugin.h"
#include "interfaces/netstoremanager/istorageaccount.h"
#include "managertab.h"
#include "xmlsettingsmanager.h"
#include "accountsmanager.h"
#include "accountslistwidget.h"
#include "upmanager.h"
#include "syncmanager.h"

namespace LC
{
namespace NetStoreManager
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

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

		qRegisterMetaType<SyncerInfo> ("SyncerInfo");
		qRegisterMetaTypeStreamOperators<SyncerInfo> ("SyncerInfo");
		qRegisterMetaTypeStreamOperators<QList<SyncerInfo>> ("QList<SyncerInfo>");
		qRegisterMetaType<Change> ("Change");
		qRegisterMetaTypeStreamOperators<Change> ("Change");
		qRegisterMetaType<StorageItem> ("StorageItem");
		qRegisterMetaTypeStreamOperators<StorageItem> ("StorageItem");

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "netstoremanagersettings.xml");

		AccountsManager_ = new AccountsManager (proxy, this);
		XSD_->SetCustomWidget ("AccountsWidget", new AccountsListWidget (AccountsManager_));

		UpManager_ = new UpManager (proxy, this);

		connect (UpManager_,
				SIGNAL (fileUploaded (QString, QUrl)),
				this,
				SIGNAL (fileUploaded (QString, QUrl)));
	}

	void Plugin::SecondInit ()
	{
		SyncManager_ = new SyncManager (AccountsManager_, this);
		SyncWidget *w = new SyncWidget (AccountsManager_);
		connect (w,
				SIGNAL (directoriesToSyncUpdated (QList<SyncerInfo>)),
				SyncManager_,
				SLOT (handleDirectoriesToSyncUpdated (QList<SyncerInfo>)));
		XSD_->SetCustomWidget ("SyncWidget", w);
		w->RestoreData ();
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
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
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
		for (auto account : AccountsManager_->GetAccounts ())
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
					Priority::Critical));
			return;
		}

		UpManager_->handleUploadRequest (account, filename);
		UpManager_->ScheduleAutoshare (filename);
	}

	QDataStream& operator<< (QDataStream& out, const SyncerInfo& info)
	{
		out << static_cast<quint8> (1)
				<< info.AccountId_
				<< info.LocalDirectory_
				<< info.RemoteDirectory_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, SyncerInfo& info)
	{
		quint8 version = 0;
		in >> version;
		if (version == 1)
			in >> info.AccountId_
					>> info.LocalDirectory_
					>> info.RemoteDirectory_;
		return in;
	}

	QDataStream& operator<< (QDataStream& out, const Change& change)
	{
		out << static_cast<quint8> (1)
				<< change.ID_
				<< change.ItemID_
				<< change.Deleted_
				<< change.Item_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, Change& change)
	{
		quint8 version = 0;
		in >> version;
		if (version == 1)
			in >> change.ID_
					>> change.ItemID_
					>> change.Deleted_
					>> change.Item_;
		return in;
	}

	QDataStream& operator<< (QDataStream& out, const StorageItem& item)
	{
		out << static_cast<quint8> (1)
				<< item.ID_
				<< item.ParentID_
				<< item.Name_
				<< item.IsDirectory_
				<< item.Hash_
				<< item.ModifyDate_
				<< item.Size_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, StorageItem& item)
	{
		quint8 version = 0;
		in >> version;
		if (version == 1)
			in >> item.ID_
					>> item.ParentID_
					>> item.Name_
					>> item.IsDirectory_
					>> item.Hash_
					>> item.ModifyDate_
					>> item.Size_;
		return in;
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_netstoremanager, LC::NetStoreManager::Plugin);
