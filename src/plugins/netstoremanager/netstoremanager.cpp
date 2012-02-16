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

#include "netstoremanager.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include "interfaces/netstoremanager/istorageplugin.h"
#include "managertab.h"
#include "xmlsettingsmanager.h"
#include "accountsmanager.h"
#include "accountslistwidget.h"
#include "upmanager.h"

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

		UpManager_ = new UpManager (this);

		connect (UpManager_,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.NetStoreManager";
	}

	void Plugin::Release ()
	{
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
		return QIcon (":/netstoremanager/resources/images/netstoremanager.svg");
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return TabClasses_t () << ManagerTC_;
	}

	void Plugin::TabOpenRequested (const QByteArray& id)
	{
		if (id == ManagerTC_.TabClass_)
		{
			ManagerTab *tab = new ManagerTab (ManagerTC_, AccountsManager_, this);
			emit addNewTab (tr ("Net storage"), tab);
			emit changeTabIcon (tab, GetIcon ());
			emit raiseTab (tab);
			connect (tab,
					SIGNAL (removeTab (QWidget*)),
					this,
					SIGNAL (removeTab (QWidget*)));
			connect (tab,
					SIGNAL (uploadRequested (IStorageAccount*, QString)),
					UpManager_,
					SLOT (handleUploadRequest (IStorageAccount*, QString)));
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
		qDebug () << Q_FUNC_INFO << pluginObj;
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
}
}

LC_EXPORT_PLUGIN (leechcraft_netstoremanager, LeechCraft::NetStoreManager::Plugin);
