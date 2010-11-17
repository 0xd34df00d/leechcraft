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

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			AccountsListDialog::AccountsListDialog (QWidget* parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);
				QMenu *addMenu = new QMenu (tr ("Add account"));

				addMenu->addActions (Core::Instance ().GetAccountCreatorActions ());

				Ui_.Add_->setMenu (addMenu);

				QStandardItemModel *accModel = new QStandardItemModel ();
				Q_FOREACH (Plugins::IAccount *acc,
						Core::Instance ().GetAccounts ())
				{
					QStandardItem *item = new QStandardItem (acc->GetAccountName ());
					item->setData (QVariant::fromValue<Plugins::IAccount*> (acc));
					item->setEditable (false);
					accModel->appendRow (item);
				}
				Ui_.Accounts_->setModel (accModel);
			}
		}
	}
}
