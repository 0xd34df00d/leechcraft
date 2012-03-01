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

#include "accountslistwidget.h"
#include "addaccountdialog.h"
#include "accountsmanager.h"
#include "interfaces/netstoremanager/istorageplugin.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	AccountsListWidget::AccountsListWidget (AccountsManager *manager, QWidget *parent)
	: QWidget (parent)
	, Manager_ (manager)
	{
		Ui_.setupUi (this);

		Ui_.Accounts_->setModel (manager->GetModel ());
	}

	void AccountsListWidget::on_Add__released ()
	{
		const auto& plugins = Manager_->GetPlugins ();

		AddAccountDialog dia (plugins, this);
		if (dia.exec () != QDialog::Accepted)
			return;

		auto plug = dia.GetStoragePlugin ();
		const QString& name = dia.GetAccountName ();
		if (!plug || name.isEmpty ())
			return;

		plug->RegisterAccount (name);
	}

	void AccountsListWidget::on_Remove__released ()
	{
		const QModelIndex& cur = Ui_.Accounts_->currentIndex ();
		if (!cur.isValid ())
			return;

		Manager_->RemoveAccount (cur);
	}
}
}
