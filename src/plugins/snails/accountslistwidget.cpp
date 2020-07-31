/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountslistwidget.h"
#include "account.h"
#include "accountsmanager.h"

namespace LC
{
namespace Snails
{
	AccountsListWidget::AccountsListWidget (AccountsManager *accsMgr, QWidget *parent)
	: QWidget { parent }
	, AccsMgr_ { accsMgr }
	{
		Ui_.setupUi (this);

		Ui_.AccountsTree_->setModel (AccsMgr_->GetAccountsModel ());
	}

	void AccountsListWidget::on_AddButton__released ()
	{
		AccsMgr_->InitiateAccountAddition ();
	}

	void AccountsListWidget::on_ModifyButton__released ()
	{
		const auto& current = Ui_.AccountsTree_->currentIndex ();
		if (!current.isValid ())
			return;

		const auto& acc = AccsMgr_->GetAccount (current);
		if (!acc)
			return;

		acc->OpenConfigDialog ();
	}

	void AccountsListWidget::on_RemoveButton__released ()
	{
	}
}
}
