/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountsselectdialog.h"
#include <QStandardItemModel>
#include "interfaces/blogique/iaccount.h"
#include "interfaces/blogique/ibloggingplatform.h"

namespace LC
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
			auto ibp = qobject_cast<IBloggingPlatform*> (acc->GetParentBloggingPlatform ());
			if (!ibp ||
					!(ibp->GetFeatures () & IBloggingPlatform::BPFSupportsBackup))
				continue;

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
				accounts << Item2Accotun_ [item];
		}

		return accounts;
	}

}
}
