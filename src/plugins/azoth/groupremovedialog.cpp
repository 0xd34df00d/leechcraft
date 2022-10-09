/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "groupremovedialog.h"
#include <QStandardItemModel>
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/iaccount.h"

namespace LC
{
namespace Azoth
{
	GroupRemoveDialog::GroupRemoveDialog (const QList<ICLEntry*>& entries, QWidget *parent)
	: QDialog (parent)
	, Entries_ (entries)
	, Model_ (new QStandardItemModel (this))
	{
		Model_->setHorizontalHeaderLabels ({ tr ("Name"), tr ("ID") });
		for (auto entry : entries)
		{
			auto nameItem = new QStandardItem (entry->GetEntryName ());
			nameItem->setCheckable (true);
			nameItem->setEditable (false);
			nameItem->setCheckState (Qt::Checked);

			auto idItem = new QStandardItem (entry->GetHumanReadableID ());
			idItem->setEditable (false);

			Model_->appendRow ({ nameItem, idItem });
		}

		Ui_.setupUi (this);
		Ui_.View_->setModel (Model_);
	}

	QList<ICLEntry*> GroupRemoveDialog::GetSelectedEntries () const
	{
		QList<ICLEntry*> result;
		for (auto i = 0; i < Model_->rowCount (); ++i)
			if (Model_->item (i)->checkState () == Qt::Checked)
				result << Entries_.at (i);
		return result;
	}

	void GroupRemoveDialog::accept ()
	{
		for (auto entry : GetSelectedEntries ())
			entry->GetParentAccount ()->RemoveEntry (entry->GetQObject ());

		QDialog::accept ();
	}
}
}
