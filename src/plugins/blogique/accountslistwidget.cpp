/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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
#include <QMessageBox>
#include <QStandardItemModel>
#include <QtDebug>
#include <QWizard>
#include "interfaces/blogique/ibloggingplatform.h"
#include "interfaces/blogique/iaccount.h"
#include "addaccountwizardfirstpage.h"
#include "core.h"

namespace LeechCraft
{
namespace Blogique
{
	AccountsListWidget::AccountsListWidget (QWidget* parent)
	: QWidget (parent)
	, AccountsModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		connect (&Core::Instance (),
				SIGNAL (accountAdded (IAccount*)),
				this,
				SLOT (addAccount (IAccount*)));
		connect (&Core::Instance (),
				SIGNAL (accountRemoved (IAccount*)),
				this,
				SLOT (handleAccountRemoved (IAccount*)));

		Q_FOREACH (IAccount *acc, Core::Instance ().GetAccounts ())
			addAccount (acc);

		Ui_.Accounts_->setModel (AccountsModel_);
	}

	void AccountsListWidget::addAccount (IAccount *acc)
	{
		IBloggingPlatform *ibp = qobject_cast<IBloggingPlatform*> (acc->
				GetParentBloggingPlatform ());

		QStandardItem *item = new QStandardItem (acc->GetAccountName ());
		item->setIcon (ibp ? ibp->GetBloggingPlatformIcon () : QIcon ());
		item->setEditable (false);
		AccountsModel_->appendRow (item);

		Item2Account_ [item] = acc;
		Account2Item_ [acc] = item;
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

		QStandardItem *item = Account2Item_ [acc];
		Item2Account_.remove (item);
		AccountsModel_->removeRow (item->row ());
		Account2Item_.remove (acc);
	}

	void AccountsListWidget::on_Add__released ()
	{
		QWizard *wizard = new QWizard (this);
		wizard->setAttribute (Qt::WA_DeleteOnClose);
		wizard->setWindowTitle (QObject::tr ("Add account"));
		wizard->addPage (new AddAccountWizardFirstPage (wizard));

		wizard->show ();
	}

	void AccountsListWidget::on_Modify__released ()
	{
		QModelIndex index = Ui_.Accounts_->selectionModel ()->currentIndex ();
		if (!index.isValid ())
			return;

		QStandardItem *item = AccountsModel_->itemFromIndex (index);
		if (item &&
				Item2Account_.contains (item))
			Item2Account_ [item]->OpenConfigurationDialog ();
	}

	void AccountsListWidget::on_Delete__released ()
	{
		QModelIndex index = Ui_.Accounts_->selectionModel ()->currentIndex ();
		if (!index.isValid ())
			return;

		QStandardItem *item = AccountsModel_->itemFromIndex (index);
		IAccount *acc = 0;
		if (item &&
				Item2Account_.contains (item))
			acc = Item2Account_ [item];
		else
			return;

		if (QMessageBox::question (this,
				"LeechCraft",
				tr ("Are you sure you want to remove the account %1?")
						.arg (acc->GetAccountName ()),
				QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		QObject *bpObj = acc->GetParentBloggingPlatform ();
		IBloggingPlatform *ibp = qobject_cast<IBloggingPlatform*> (bpObj);
		if (!ibp)
		{
			qWarning () << Q_FUNC_INFO
					<< "parent blogging platform for"
					<< acc->GetAccountID ()
					<< "doesn't implement IBloggingPlatform";
			return;
		}
		ibp->RemoveAccount (acc->GetObject ());
	}
}
}
