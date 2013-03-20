/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
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

#include "submittodialog.h"
#include <QStandardItemModel>
#include "interfaces/blogique/iaccount.h"
#include "interfaces/blogique/ibloggingplatform.h"
#include "core.h"
#include "selecttargetdelegate.h"

namespace LeechCraft
{
namespace Blogique
{
	SubmitToDialog::SubmitToDialog (QWidget *parent)
	: QDialog (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		Model_->setHorizontalHeaderLabels ({tr ("Account"), tr ("Target") });
		Ui_.TargetsView_->setModel (Model_);

		Ui_.TargetsView_->setItemDelegateForColumn (SubmitToDialog::Target, new SelectTargetDelegate (this));

		for (auto acc : Core::Instance ().GetAccounts ())
		{
			QStandardItem *accountItem = new QStandardItem (acc->GetAccountName ());
			accountItem->setEditable (false);
			accountItem->setCheckable (true);

			QStandardItem *targetItem = new QStandardItem;

			Model_->appendRow ({ accountItem, targetItem });
			Ui_.TargetsView_->resizeColumnToContents (Account);
			Item2Account_ [accountItem] = acc;

			Ui_.TargetsView_->openPersistentEditor (Model_->indexFromItem (targetItem));
		}
	}

	IAccount* SubmitToDialog::GetAccountFromIndex (const QModelIndex& index) const
	{
		return Item2Account_.value (Model_->itemFromIndex (index));
	}

	QAbstractItemModel* SubmitToDialog::GetModel () const
	{
		return Model_;
	}

	QList<QPair<IAccount*, QString>> SubmitToDialog::GetPostingTargets () const
	{
		QList<QPair<IAccount*, QString>> result;
		for (int i = 0; i < Model_->rowCount (); ++i)
		{
			auto accItem = Model_->item (i, Account);
			auto targetItem = Model_->item (i, Target);
			if (accItem->checkState () != Qt::Checked)
				continue;

			auto account = Item2Account_.value (accItem);
			auto ibp = qobject_cast<IBloggingPlatform*> (account->GetParentBloggingPlatform ());
			if (!ibp)
				continue;

			QString target;
			if (ibp->GetFeatures () & IBloggingPlatform::BPFSelectablePostDestination)
				target = targetItem->data (SelectTargetDelegate::TargetRole).toString ();
			result << qMakePair<IAccount*, QString> (account, target);
		}

		return result;
	}

}
}
