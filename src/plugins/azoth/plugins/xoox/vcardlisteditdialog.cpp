/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vcardlisteditdialog.h"
#include <QStandardItemModel>
#include <QInputDialog>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	VCardListEditDialog::VCardListEditDialog (const QStringList& options,
			QWidget *parent)
	: QDialog (parent)
	, Model_ (new QStandardItemModel (this))
	{
		QStringList headers (tr ("Item"));
		headers << options;
		Model_->setHorizontalHeaderLabels (headers);

		Ui_.setupUi (this);
		Ui_.Items_->setModel (Model_);
	}

	void VCardListEditDialog::AddItems (const QList<QPair<QString, QStringList>>& items)
	{
		const int rc = Model_->columnCount ();
		for (const auto& item : items)
		{
			QList<QStandardItem*> row { new QStandardItem (item.first) };

			const auto& opts = item.second;
			for (int i = 1; i < rc; ++i)
			{
				const bool checked = opts.contains (Model_->horizontalHeaderItem (i)->text ());
				auto item = new QStandardItem;
				item->setCheckable (true);
				item->setCheckState (checked ? Qt::Checked : Qt::Unchecked);
				row << item;
			}

			Model_->appendRow (row);
		}
	}

	QList<QPair<QString, QStringList>> VCardListEditDialog::GetItems () const
	{
		QList<QPair<QString, QStringList>> result;

		const int rc = Model_->columnCount ();
		for (int i = 0, rows = Model_->rowCount (); i < rows; ++i)
		{
			const auto& text = Model_->item (i, 0)->text ();
			QStringList options;
			for (int j = 1; j < rc; ++j)
			{
				if (Model_->item (i, j)->checkState () == Qt::Checked)
					options << Model_->horizontalHeaderItem (j)->text ();
			}
			result << qMakePair (text, options);
		}

		return result;
	}

	void VCardListEditDialog::on_Add__released ()
	{
		const auto& str = QInputDialog::getText (this,
				"LeechCraft",
				tr ("Enter the new item:"));
		if (str.isEmpty ())
			return;

		AddItems ({ { str, {} } });
	}

	void VCardListEditDialog::on_Remove__released ()
	{
		const auto& idx = Ui_.Items_->currentIndex ();
		if (!idx.isValid ())
			return;

		Model_->removeRow (idx.row ());
	}
}
}
}
