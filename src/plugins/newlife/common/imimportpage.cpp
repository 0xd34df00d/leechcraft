/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "imimportpage.h"
#include <QStandardItemModel>

namespace LeechCraft
{
namespace NewLife
{
namespace Common
{
	QObject *IMImportPage::S_Plugin_ = 0;

	void IMImportPage::SetPluginInstance (QObject *obj)
	{
		S_Plugin_ = obj;
	}

	IMImportPage::IMImportPage (QWidget *parent)
	: QWizardPage (parent)
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
		connect (this,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				wizard (),
				SIGNAL (gotEntity (LeechCraft::Entity)));

		AccountsModel_->clear ();

		QStringList labels;
		labels << tr ("Account name")
				<< tr ("JID")
				<< tr ("Import account settings")
				<< tr ("Import history");
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
