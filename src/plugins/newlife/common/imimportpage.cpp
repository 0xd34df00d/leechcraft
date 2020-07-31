/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "imimportpage.h"
#include <QStandardItemModel>

namespace LC
{
namespace NewLife
{
namespace Common
{
	IMImportPage::IMImportPage (const ICoreProxy_ptr& proxy, QWidget *parent)
	: EntityGeneratingPage (proxy, parent)
	, AccountsModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.AccountsTree_->setModel (AccountsModel_);
	}

	bool IMImportPage::isComplete () const
	{
		return true;
	}

	int IMImportPage::nextId () const
	{
		return -1;
	}

	void IMImportPage::initializePage ()
	{
		connect (wizard (),
				SIGNAL (accepted ()),
				this,
				SLOT (handleAccepted ()),
				Qt::UniqueConnection);

		AccountsModel_->clear ();

		const QStringList labels
		{
			tr ("Account name"),
			tr ("JID"),
			tr ("Import account settings"),
			tr ("Import history")
		};
		AccountsModel_->setHorizontalHeaderLabels (labels);

		FindAccounts ();
	}

	void IMImportPage::handleAccepted ()
	{
		for (int i = 0; i < AccountsModel_->rowCount (); ++i)
		{
			QStandardItem *profItem = AccountsModel_->item (i);
			for (int j = 0; j < profItem->rowCount (); ++j)
			{
				QStandardItem *accItem = profItem->child (j);
				if (profItem->child (j, Column::ImportAcc)->checkState () == Qt::Checked)
					SendImportAcc (accItem);
				if (profItem->child (j, Column::ImportHist)->checkState () == Qt::Checked)
					SendImportHist (accItem);
			}
		}
	}
}
}
}
