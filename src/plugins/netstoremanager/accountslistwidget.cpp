/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountslistwidget.h"
#include "addaccountdialog.h"
#include "accountsmanager.h"
#include "interfaces/netstoremanager/istorageplugin.h"

namespace LC
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
