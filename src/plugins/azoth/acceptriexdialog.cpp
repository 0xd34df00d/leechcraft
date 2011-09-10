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

#include "acceptriexdialog.h"
#include <QStandardItemModel>
#include "interfaces/iclentry.h"

namespace LeechCraft
{
namespace Azoth
{
	AcceptRIEXDialog::AcceptRIEXDialog (const QList<RIEXItem>& items,
			QObject *entryObj, QString message, QWidget *parent)
	: QDialog (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		QStringList labels;
		labels << tr ("Action")
				<< tr ("ID")
				<< tr ("Name")
				<< tr ("Groups");
		Model_->setHorizontalHeaderLabels (labels);

		Q_FOREACH (const RIEXItem& item, items)
		{
			QList<QStandardItem*> row;

			QStandardItem *action = new QStandardItem;
			action->setCheckState (Qt::Checked);
			action->setCheckable (true);
			switch (item.Action_)
			{
			case RIEXItem::AAdd:
				action->setText (tr ("add"));
				break;
			case RIEXItem::ADelete:
				action->setText (tr ("delete"));
				break;
			case RIEXItem::AModify:
				action->setText (tr ("modify"));
				break;
			default:
				action->setText (tr ("(unknown)"));
				break;
			}

			action->setData (QVariant::fromValue<RIEXItem> (item));

			row << action;
			row << new QStandardItem (item.ID_);
			row << new QStandardItem (item.Nick_);
			row << new QStandardItem (item.Groups_.join ("; "));

			Model_->appendRow (row);
		}

		Ui_.ItemsTree_->setModel (Model_);

		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		const QString& id = entry->GetEntryName ().isEmpty () ?
				entry->GetHumanReadableID () :
				entry->GetEntryName () + " (" + entry->GetHumanReadableID () + ")";

		const QString& text = message.isEmpty () ?
				tr ("%1 has suggested to modify your contact list:")
					.arg (id) :
				tr ("%1 has suggested to modify your contact list:\n%2")
					.arg (id)
					.arg (message);
		Ui_.MessageLabel_->setText (text);
	}

	QList<RIEXItem> AcceptRIEXDialog::GetSelectedItems () const
	{
		QList<RIEXItem> result;

		for (int i = 0, size = Model_->rowCount (); i < size; ++i)
		{
			QStandardItem *item = Model_->item (i);
			if (item->checkState () != Qt::Checked)
				continue;

			result << item->data ().value<RIEXItem> ();
		}

		return result;
	}
}
}
