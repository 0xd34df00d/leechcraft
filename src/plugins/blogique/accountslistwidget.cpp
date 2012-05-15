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
	AccountsListWidget::AccountsListWidget (QWidget *parent)
	: QWidget (parent)
	, AccountsModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		connect (&Core::Instance (),
				SIGNAL (accountAdded (QObject*)),
				this,
				SLOT (addAccount (QObject*)));
		connect (&Core::Instance (),
				SIGNAL (accountRemoved (QObject*)),
				this,
				SLOT (handleAccountRemoved (QObject*)));
		connect (&Core::Instance (),
				SIGNAL (accountValidated (QObject*, bool)),
				this,
				SLOT (handleAccountValidated (QObject*, bool)));

		Q_FOREACH (IAccount *acc, Core::Instance ().GetAccounts ())
			addAccount (acc->GetObject ());

		AccountsModel_->setHorizontalHeaderLabels ({tr ("Account"), tr ("Validated")});
		Ui_.Accounts_->setModel (AccountsModel_);
	}

	void AccountsListWidget::addAccount (QObject *accObj)
	{
		IAccount *acc = qobject_cast<IAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< accObj
					<< "is not an IAccount";
			return;
		}

		IBloggingPlatform *ibp = qobject_cast<IBloggingPlatform*> (acc->
				GetParentBloggingPlatform ());

		QStandardItem *item = new QStandardItem (acc->GetAccountName ());
		item->setIcon (ibp ? ibp->GetBloggingPlatformIcon () : QIcon ());
		item->setEditable (false);
		QStandardItem *itemValidated = new QStandardItem (acc->IsValidated () ?
				tr ("Validated") :
				tr ("Not validated"));
		itemValidated->setEditable (false);
		AccountsModel_->appendRow ({item, itemValidated});

		Ui_.Accounts_->header ()->setResizeMode (QHeaderView::ResizeToContents);

		Item2Account_ [item] = acc;
		Account2Item_ [acc] = item;
	}

	void AccountsListWidget::handleAccountRemoved (QObject *accObj)
	{
		IAccount *acc = qobject_cast<IAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< accObj
					<< "is not an IAccount";
			return;
		}

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

	void AccountsListWidget::handleAccountValidated (QObject *accObj, bool validated)
	{
		IAccount *acc = qobject_cast<IAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< accObj
					<< "is not an IAccount";
			return;
		}

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
		AccountsModel_->item (item->row (), Columns::IsValidated)->setText (validated ?
				tr ("Validated") :
				tr ("Not validated"));
		Ui_.Accounts_->header ()->setResizeMode (QHeaderView::ResizeToContents);
	}

	void AccountsListWidget::on_Add__released ()
	{
		QWizard *wizard = new QWizard (this);
		wizard->setAttribute (Qt::WA_DeleteOnClose);
		wizard->setWindowTitle (tr ("Add account"));
		wizard->addPage (new AddAccountWizardFirstPage (wizard));

		wizard->show ();
	}

	void AccountsListWidget::on_Modify__released ()
	{
		auto index = Ui_.Accounts_->selectionModel ()->currentIndex ();
		index = index.sibling (index.row (), 0);
		if (!index.isValid ())
			return;

		QStandardItem *item = AccountsModel_->itemFromIndex (index);
		if (item &&
				Item2Account_.contains (item))
			Item2Account_ [item]->OpenConfigurationDialog ();
	}

	void AccountsListWidget::on_Delete__released ()
	{
		auto index = Ui_.Accounts_->selectionModel ()->currentIndex ();
		index = index.sibling (index.row (), 0);
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
						.arg ("<em>" + acc->GetAccountName () + "</em>"),
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
