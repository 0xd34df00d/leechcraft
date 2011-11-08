/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "managecontactsdialog.h"
#include <QStandardItemModel>
#include <QMessageBox>
#include <QtDebug>
#include <interfaces/iclentry.h>
#include <interfaces/iaccount.h>
#include <interfaces/iprotocol.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Metacontacts
{
	ManageContactsDialog::ManageContactsDialog (const QList<QObject*>& objects, QWidget *parent)
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

		Q_FOREACH (QObject *entryObj, objects)
		{
			ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
			if (!entry)
			{
				qWarning () << Q_FUNC_INFO
						<< entryObj
						<< "not a ICLEntry";
				continue;
			}

			IAccount *account = qobject_cast<IAccount*> (entry->GetParentAccount ());
			IProtocol *proto = qobject_cast<IProtocol*> (account->GetParentProtocol ());

			if (!account || !proto)
			{
				qWarning () << Q_FUNC_INFO
						<< "invalid parent chain for"
						<< entryObj;
				continue;
			}

			QList<QStandardItem*> row;

			QStandardItem *nameItem = new QStandardItem (entry->GetEntryName ());
			nameItem->setData (QVariant::fromValue<QObject*> (entryObj));
			row << nameItem;

			row << new QStandardItem (entry->GetHumanReadableID ());
			row << new QStandardItem (account->GetAccountName ());

			QStandardItem *protoItem = new QStandardItem (proto->GetProtocolName ());
			protoItem->setIcon (proto->GetProtocolIcon ());
			row << protoItem;

			Model_->appendRow (row);
		}
	}

	QList<QObject*> ManageContactsDialog::GetObjects () const
	{
		QList<QObject*> result;
		for (int i = 0; i < Model_->rowCount (); ++i)
			result << Model_->item (i)->data ().value<QObject*> ();
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
