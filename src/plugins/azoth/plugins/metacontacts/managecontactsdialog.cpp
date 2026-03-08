/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "managecontactsdialog.h"
#include <QStandardItemModel>
#include <QMessageBox>
#include <QtDebug>
#include <util/sll/qobjectrefcast.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iprotocol.h>

namespace LC
{
namespace Azoth
{
namespace Metacontacts
{
	ManageContactsDialog::ManageContactsDialog (const QList<ICLEntry*>& entries, QWidget *parent)
	: QDialog (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.ContactsTree_->setModel (Model_);

		QStringList labels;
		labels << tr ("Name")
				<< tr ("ID")
				<< tr ("Account")
				<< tr ("Protocol");
		Model_->setHorizontalHeaderLabels (labels);

		for (const auto entry : entries)
		{
			const auto account = entry->GetParentAccount ();
			const auto& proto = qobject_ref_cast<IProtocol> (account->GetParentProtocol ());

			QList<QStandardItem*> row;

			QStandardItem *nameItem = new QStandardItem (entry->GetEntryName ());
			nameItem->setData (QVariant::fromValue (entry));
			row << nameItem;

			row << new QStandardItem (entry->GetHumanReadableID ());
			row << new QStandardItem (account->GetAccountName ());

			QStandardItem *protoItem = new QStandardItem (proto.GetProtocolName ());
			protoItem->setIcon (proto.GetProtocolIcon ());
			row << protoItem;

			Model_->appendRow (row);
		}
	}

	QList<ICLEntry*> ManageContactsDialog::GetObjects () const
	{
		QList<ICLEntry*> result;
		for (int i = 0; i < Model_->rowCount (); ++i)
			result << Model_->item (i)->data ().value<ICLEntry*> ();
		return result;
	}

	void ManageContactsDialog::on_MoveUp__released ()
	{
		const QModelIndex& index = Ui_.ContactsTree_->currentIndex ();
		const int row = index.row ();
		if (!index.isValid () ||
				!row)
			return;

		Model_->insertRow (row - 1, Model_->takeRow (row));
	}

	void ManageContactsDialog::on_MoveDown__released ()
	{
		const QModelIndex& index = Ui_.ContactsTree_->currentIndex ();
		const int row = index.row ();
		if (!index.isValid () ||
				row == Model_->rowCount () - 1)
			return;

		Model_->insertRow (row, Model_->takeRow (row + 1));
	}

	void ManageContactsDialog::on_Remove__released ()
	{
		const QModelIndex& index = Ui_.ContactsTree_->currentIndex ();
		if (!index.isValid ())
			return;

		const QModelIndex& trueIndex = index.sibling (index.row (), 0);
		if (QMessageBox::question (0,
					"LeechCraft",
					tr ("Are you sure you want to remove %1 from "
						"this metacontact?")
						.arg (trueIndex.data ().toString ()),
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		Model_->removeRow (trueIndex.row ());
	}
}
}
}
