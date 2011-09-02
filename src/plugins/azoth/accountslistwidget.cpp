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

#include "accountslistwidget.h"
#include <QMenu>
#include <QWizard>
#include <QMessageBox>
#include <QStandardItemModel>
#include "interfaces/iaccount.h"
#include "interfaces/iprotocol.h"
#ifdef ENABLE_CRYPT
#include "interfaces/isupportpgp.h"
#include "pgpkeyselectiondialog.h"
#endif
#include "core.h"
#include "util.h"

namespace LeechCraft
{
namespace Azoth
{
	AccountsListWidget::AccountsListWidget (QWidget* parent)
	: QWidget (parent)
	, AccModel_ (new QStandardItemModel ())
	{
		Ui_.setupUi (this);

#ifdef ENABLE_CRYPT
		Ui_.PGP_->setEnabled (true);
#endif

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

	void AccountsListWidget::addAccount (IAccount *acc)
	{
		IProtocol *proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());

		QStandardItem *item = new QStandardItem (acc->GetAccountName ());
		item->setIcon (proto ? proto->GetProtocolIcon () : QIcon ());
		item->setData (QVariant::fromValue<IAccount*> (acc), RAccObj);
		item->setEditable (false);
		AccModel_->appendRow (item);

		Account2Item_ [acc] = item;
	}

	void AccountsListWidget::on_Add__released ()
	{
		InitiateAccountAddition (this);
	}

	void AccountsListWidget::on_Modify__released ()
	{
		QModelIndex index = Ui_.Accounts_->
				selectionModel ()->currentIndex ();
		if (!index.isValid ())
			return;

		IAccount *acc = index
				.data (RAccObj).value<IAccount*> ();
		acc->OpenConfigurationDialog ();
	}

	void AccountsListWidget::on_PGP__released ()
	{
#ifdef ENABLE_CRYPT
		QModelIndex index = Ui_.Accounts_->
				selectionModel ()->currentIndex ();
		if (!index.isValid ())
			return;

		IAccount *acc = index
				.data (RAccObj).value<IAccount*> ();
		ISupportPGP *pgpAcc = qobject_cast<ISupportPGP*> (acc->GetObject ());
		if (!pgpAcc)
		{
			QMessageBox::warning (this,
					"LeechCraft",
					tr ("The account %1 doesn't support encryption.")
						.arg (acc->GetAccountName ()));
			return;
		}

		const QString& str = tr ("Please select new PGP key for the account %1.")
				.arg (acc->GetAccountName ());
		PGPKeySelectionDialog dia (str, PGPKeySelectionDialog::TPrivate, this);
		if (dia.exec () != QDialog::Accepted)
			return;

		pgpAcc->SetPrivateKey (dia.GetSelectedKey ());
		Core::Instance ().AssociatePrivateKey (acc, dia.GetSelectedKey ());
#endif
	}

	void AccountsListWidget::on_Delete__released()
	{
		QModelIndex index = Ui_.Accounts_->
		selectionModel ()->currentIndex ();
		if (!index.isValid ())
			return;

		IAccount *acc = index
				.data (RAccObj).value<IAccount*> ();

		if (QMessageBox::question (this,
					"LeechCraft",
					tr ("Are you sure you want to remove the account %1?")
						.arg (acc->GetAccountName ()),
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

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

	void AccountsListWidget::handleAccountRemoved (IAccount *acc)
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
