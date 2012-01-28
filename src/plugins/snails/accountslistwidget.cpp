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
#include "account.h"
#include "core.h"

namespace LeechCraft
{
namespace Snails
{
	AccountsListWidget::AccountsListWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);

		Ui_.AccountsTree_->setModel (Core::Instance ().GetAccountsModel ());
	}

	void AccountsListWidget::on_AddButton__released ()
	{
		Account_ptr acc (new Account);

		acc->OpenConfigDialog ();

		if (acc->IsNull ())
			return;

		Core::Instance ().AddAccount (acc);
	}

	void AccountsListWidget::on_ModifyButton__released ()
	{
		const QModelIndex& current = Ui_.AccountsTree_->currentIndex ();
		if (!current.isValid ())
			return;

		Account_ptr acc = Core::Instance ().GetAccount (current);
		if (!acc)
			return;

		acc->OpenConfigDialog ();
	}

	void AccountsListWidget::on_RemoveButton__released ()
	{
	}
}
}
