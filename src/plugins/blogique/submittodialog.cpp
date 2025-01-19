/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "submittodialog.h"
#include <QStandardItemModel>
#include "interfaces/blogique/iaccount.h"
#include "interfaces/blogique/ibloggingplatform.h"
#include "core.h"
#include "selecttargetdelegate.h"

namespace LC
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
			result << QPair { account, target };
		}

		return result;
	}

}
}
