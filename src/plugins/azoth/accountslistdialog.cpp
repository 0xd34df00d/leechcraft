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

#include "accountslistdialog.h"
#include <QMenu>
#include <QWizard>
#include <QStandardItemModel>
#include "interfaces/iaccount.h"
#include "interfaces/iprotocol.h"
#include "core.h"
#include "addaccountwizardfirstpage.h"

namespace LeechCraft
{
namespace Azoth
{
	AccountsListDialog::AccountsListDialog (QWidget* parent)
	: QDialog (parent)
	, AccModel_ (new QStandardItemModel ())
	{
		Ui_.setupUi (this);
		QMenu *addMenu = new QMenu (tr ("Add account"));

		addMenu->addActions (Core::Instance ().GetAccountCreatorActions ());

		//Ui_.Add_->setMenu (addMenu);

		connect (&Core::Instance (),
				SIGNAL (accountAdded (IAccount*)),
				this,
				SLOT (addAccount (IAccount*)));
		connect (&Core::Instance (),
				SIGNAL (accountRemoved (IAccount*)),
				this,
				SLOT (handleAccountRemoved (IAccount*)));

		Q_FOREACH (IAccount *acc,
				Core::Instance ().GetAccounts ())
			addAccount (acc);

		Ui_.Accounts_->setModel (AccModel_);
	}

	void AccountsListDialog::addAccount (IAccount *acc)
	{
		QStandardItem *item = new QStandardItem (acc->GetAccountName ());
		item->setData (QVariant::fromValue<IAccount*> (acc), RAccObj);
		item->setEditable (false);
		AccModel_->appendRow (item);

		Account2Item_ [acc] = item;
	}
	
	void AccountsListDialog::on_Add__released ()
	{
		QWizard *wizard = new QWizard (this);
		wizard->setWindowTitle (tr ("Add account"));
		wizard->addPage (new AddAccountWizardFirstPage (wizard));
		
		wizard->show ();
	}

	void AccountsListDialog::on_Modify__released ()
	{
		QModelIndex index = Ui_.Accounts_->
				selectionModel ()->currentIndex ();
		if (!index.isValid ())
			return;

		IAccount *acc = index
				.data (RAccObj).value<IAccount*> ();
		acc->OpenConfigurationDialog ();
	}

	void AccountsListDialog::on_Delete__released()
	{
		QModelIndex index = Ui_.Accounts_->
		selectionModel ()->currentIndex ();
		if (!index.isValid ())
			return;

		IAccount *acc = index
				.data (RAccObj).value<IAccount*> ();
		QObject *protoObj = acc->GetParentProtocol ();
		IProtocol *proto = qobject_cast<IProtocol*> (protoObj);
		if (!proto)
		{
			qWarning () << Q_FUNC_INFO
					<< "parent protocol for"
					<< acc->GetAccountID ()
					<< "doesn't implement IProtocol";
			return;
		}
		proto->RemoveAccount (acc->GetObject ());
	}

	void AccountsListDialog::handleAccountRemoved (IAccount *acc)
	{
		if (!Account2Item_.contains (acc))
		{
			qWarning () << Q_FUNC_INFO
					<< "account"
					<< acc->GetAccountName ()
					<< acc->GetObject ()
					<< "from"
					<< sender ()
					<< "not found here";
			return;
		}

		AccModel_->removeRow (Account2Item_ [acc]->row ());
		Account2Item_.remove (acc);
	}
}
}
