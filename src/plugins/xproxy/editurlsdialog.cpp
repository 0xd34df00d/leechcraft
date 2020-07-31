/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "editurlsdialog.h"
#include <QStandardItemModel>
#include <util/sll/slotclosure.h>
#include "editurldialog.h"

namespace LC
{
namespace XProxy
{
	namespace
	{
		QList<QStandardItem*> ReqTarget2Row (const ReqTarget& item)
		{
			QList<QStandardItem*> row
			{
				new QStandardItem { item.Host_.GetPattern () },
				new QStandardItem { item.Port_ > 0 ? QString::number (item.Port_) : "any" },
				new QStandardItem { item.Protocols_.isEmpty () ? "any" : item.Protocols_.join (" ") }
			};
			for (auto rowItem : row)
				rowItem->setEditable (false);
			return row;
		}
	}

	EditUrlsDialog::EditUrlsDialog (const QList<ReqTarget>& items, QWidget *parent)
	: QDialog { parent }
	, Items_ { items }
	, Model_ { new QStandardItemModel { this } }
	{
		Ui_.setupUi (this);

		Model_->setHorizontalHeaderLabels ({ tr ("Host"), tr ("Port"), tr ("Protocols") });
		for (const auto& item : items)
			Model_->appendRow (ReqTarget2Row (item));

		Ui_.UrlsView_->setModel (Model_);
	}

	const QList<ReqTarget>& EditUrlsDialog::GetTargets () const
	{
		return Items_;
	}

	void EditUrlsDialog::on_AddButton__released ()
	{
		const auto editDia = new EditUrlDialog { this };
		editDia->setAttribute (Qt::WA_DeleteOnClose);

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[this, editDia] () -> void
			{
				const auto& reqTarget = editDia->GetReqTarget ();
				Model_->appendRow (ReqTarget2Row (reqTarget));
				Items_ << reqTarget;
			},
			editDia,
			SIGNAL (accepted ()),
			editDia
		};

		editDia->show ();
	}

	void EditUrlsDialog::on_EditButton__released ()
	{
		const auto& idx = Ui_.UrlsView_->currentIndex ();
		if (!idx.isValid ())
			return;

		const auto rowIdx = idx.row ();

		const auto editDia = new EditUrlDialog { this };
		editDia->setAttribute (Qt::WA_DeleteOnClose);
		editDia->SetReqTarget (Items_.value (rowIdx));

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[this, editDia, rowIdx] () -> void
			{
				const auto& reqTarget = editDia->GetReqTarget ();
				const auto row = ReqTarget2Row (reqTarget);
				if (rowIdx < Items_.size ())
				{
					for (int i = 0; i < row.size (); ++i)
						Model_->setItem (rowIdx, i, row.at (i));
					Items_ [rowIdx] = reqTarget;
				}
				else
				{
					Model_->appendRow (row);
					Items_ << reqTarget;
				}
			},
			editDia,
			SIGNAL (accepted ()),
			editDia
		};

		editDia->show ();
	}

	void EditUrlsDialog::on_RemoveButton__released ()
	{
		const auto& idx = Ui_.UrlsView_->currentIndex ();
		if (!idx.isValid ())
			return;

		Items_.removeAt (idx.row ());
		Model_->removeRow (idx.row ());
	}
}
}
