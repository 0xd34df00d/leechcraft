/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "vcardlisteditdialog.h"
#include <QStandardItemModel>
#include <QInputDialog>

namespace LeechCraft
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
		Q_FOREACH (const auto& item, items)
		{
			QList<QStandardItem*> row;
			row << new QStandardItem (item.first);

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

		QList<QPair<QString, QStringList>> list;
		list << qMakePair (str, QStringList ());
		AddItems (list);
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
