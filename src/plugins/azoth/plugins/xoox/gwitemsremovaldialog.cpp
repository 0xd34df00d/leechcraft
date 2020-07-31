/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "gwitemsremovaldialog.h"
#include "glooxclentry.h"
#include <QStandardItemModel>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	GWItemsRemovalDialog::GWItemsRemovalDialog (const QList<GlooxCLEntry*>& items, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		auto model = new QStandardItemModel (this);
		model->setHorizontalHeaderLabels ({ tr ("Name"), tr ("JID") });
		for (auto item : items)
		{
			QList<QStandardItem*> row
			{
				new QStandardItem (item->GetEntryName ()),
				new QStandardItem (item->GetHumanReadableID ())
			};
			for (auto r : row)
				r->setEditable (false);
			model->appendRow (row);
		}

		Ui_.EntriesView_->setModel (model);
	}
}
}
}
