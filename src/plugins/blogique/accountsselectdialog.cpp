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

#include "accountsselectdialog.h"
#include "interfaces/blogique/iaccount.h"
#include <QStandardItemModel>

namespace LeechCraft
{
namespace Blogique
{
	AccountsSelectDialog::AccountsSelectDialog (QWidget *parent)
	: QDialog (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		Model_->setHorizontalHeaderLabels ({ tr ("Account name") });
		Ui_.AccountsView_->setModel (Model_);
	}

	void AccountsSelectDialog::FillAccounts (const QList<IAccount*>& accounts)
	{
		for (auto acc : accounts)
		{
			QStandardItem *item = new QStandardItem (acc->GetAccountName ());
			item->setCheckable (true);
			item->setEditable (false);
			Item2Accotun_ [item] = acc;
			Model_->appendRow (item);
		}
	}

	QList<IAccount*> AccountsSelectDialog::GetSelectedAccounts () const
	{
		QList<IAccount*> accounts;
		for (int i = 0; i < Model_->rowCount (); ++i)
		{
			auto item = Model_->item (i);
			if (item->checkState () == Qt::Checked)
				accounts <<Item2Accotun_ [item];
		}

		return accounts;
	}

}
}
