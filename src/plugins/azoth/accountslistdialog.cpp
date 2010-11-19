/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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
#include <QStandardItemModel>
#include "interfaces/iaccount.h"
#include "core.h"
#include <boost/graph/graph_concepts.hpp>
#include "interfaces/iprotocol.h"

namespace LeechCraft
{
	namespace Plugins
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

				Ui_.Add_->setMenu (addMenu);


				Q_FOREACH (Plugins::IAccount *acc,
						Core::Instance ().GetAccounts ())
					AddAccount (acc);

				Ui_.Accounts_->setModel (AccModel_);
			}

			void AccountsListDialog::AddAccount (Plugins::IAccount *acc)
			{
				QStandardItem *item = new QStandardItem (acc->GetAccountName ());
				item->setData (QVariant::fromValue<Plugins::IAccount*> (acc), RAccObj);
				item->setEditable (false);
				AccModel_->appendRow (item);

				QObject *protoObj = acc->GetParentProtocol ();
				connect (protoObj,
						SIGNAL (accountAdded (QObject*)),
						this,
						SLOT (handleAccountAdded (QObject*)),
						Qt::UniqueConnection);
				connect (protoObj,
						SIGNAL (accountRemoved (QObject*)),
						this,
						SLOT (handleAccountRemoved (QObject*)),
						Qt::UniqueConnection);

				Account2Item_ [acc] = item;
			}

			void AccountsListDialog::on_Modify__released ()
			{
				QModelIndex index = Ui_.Accounts_->
						selectionModel ()->currentIndex ();
				if (!index.isValid ())
					return;

				Plugins::IAccount *acc = index
						.data (RAccObj).value<Plugins::IAccount*> ();
				acc->OpenConfigurationDialog ();
			}

			void AccountsListDialog::on_Delete__released()
			{
				QModelIndex index = Ui_.Accounts_->
				selectionModel ()->currentIndex ();
				if (!index.isValid ())
					return;

				Plugins::IAccount *acc = index
						.data (RAccObj).value<Plugins::IAccount*> ();
				QObject *protoObj = acc->GetParentProtocol ();
				Plugins::IProtocol *proto = qobject_cast<Plugins::IProtocol*> (protoObj);;
				if (!proto)
				{
					qWarning () << Q_FUNC_INFO
							<< "parent protocol for"
							<< acc->GetAccountID ()
							<< "doesn't implement IProtocol";
					return;
				}
				proto->RemoveAccount (acc);
			}

			void AccountsListDialog::handleAccountAdded (QObject *accObj)
			{
				Plugins::IAccount *acc = qobject_cast<Plugins::IAccount*> (accObj);
				if (!acc)
				{
					qWarning () << Q_FUNC_INFO
							<< accObj
							<< "doesn't implement IAccount, got from"
							<< sender ();
					return;
				}

				AddAccount (acc);
			}

			void AccountsListDialog::handleAccountRemoved (QObject *accObj)
			{
				Plugins::IAccount *acc = qobject_cast<Plugins::IAccount*> (accObj);
				if (!acc)
				{
					qWarning () << Q_FUNC_INFO
							<< accObj
							<< "doesn't implement IAccount, got from"
							<< sender ();
					return;
				}

				if (!Account2Item_.contains (acc))
				{
					qWarning () << Q_FUNC_INFO
							<< "account"
							<< acc->GetAccountName ()
							<< accObj
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
}
