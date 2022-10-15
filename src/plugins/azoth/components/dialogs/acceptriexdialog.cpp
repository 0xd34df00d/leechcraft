/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "acceptriexdialog.h"
#include <QStandardItemModel>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "interfaces/azoth/iclentry.h"
#include "core.h"

namespace LC
{
namespace Azoth
{
	AcceptRIEXDialog::AcceptRIEXDialog (const QList<RIEXItem>& items,
			QObject *entryObj, QString message, QWidget *parent)
	: QDialog (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		Model_->setHorizontalHeaderLabels ({ tr ("Action"), tr ("ID"), tr ("Name"), tr ("Groups") });

		for (const RIEXItem& item : items)
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

		const auto itp = Core::Instance ().GetProxy ()->GetTagsManager ();
		for (int i = 0, size = Model_->rowCount (); i < size; ++i)
		{
			QStandardItem *item = Model_->item (i);
			if (item->checkState () != Qt::Checked)
				continue;

			auto riex = item->data ().value<RIEXItem> ();
			riex.Groups_ = itp->Split (Model_->item (i, Column::Groups)->text ());
			result << riex;
		}

		return result;
	}
}
}
