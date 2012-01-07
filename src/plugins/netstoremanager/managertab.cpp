/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "managertab.h"
#include <QMessageBox>
#include <QFileDialog>
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/istorageplugin.h"
#include "accountsmanager.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	ManagerTab::ManagerTab (const TabClassInfo& tc, AccountsManager *am, QObject *obj)
	: Parent_ (obj)
	, Info_ (tc)
	, AM_ (am)
	{
		Ui_.setupUi (this);

		Q_FOREACH (auto acc, AM_->GetAccounts ())
		{
			auto stP = qobject_cast<IStoragePlugin*> (acc->GetParentPlugin ());
			Ui_.AccountsBox_->addItem (stP->GetStorageIcon (),
					acc->GetAccountName (),
					QVariant::fromValue<IStorageAccount*> (acc));
		}
	}

	TabClassInfo ManagerTab::GetTabClassInfo () const
	{
		return Info_;
	}

	QObject* ManagerTab::ParentMultiTabs ()
	{
		return Parent_;
	}

	void ManagerTab::Remove ()
	{
		emit removeTab (this);
	}

	QToolBar* ManagerTab::GetToolBar () const
	{
		return 0;
	}

	void ManagerTab::on_Upload__released ()
	{
		const int accIdx = Ui_.AccountsBox_->currentIndex ();
		if (accIdx < 0)
		{
			QMessageBox::critical (this,
					tr ("Error"),
					tr ("You first need to add an account."));
			return;
		}

		const QString& filename = QFileDialog::getOpenFileName (this,
				tr ("Select file for upload"),
				QDir::homePath ());
		if (filename.isEmpty ())
			return;

		IStorageAccount *acc = Ui_.AccountsBox_->
				itemData (accIdx).value<IStorageAccount*> ();
		emit uploadRequested (acc, filename);
	}
}
}
