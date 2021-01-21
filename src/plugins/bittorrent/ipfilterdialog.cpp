/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ipfilterdialog.h"
#include <util/sll/qtutil.h>
#include "core.h"
#include "banpeersdialog.h"

namespace LC::BitTorrent
{
	const int BlockRole = Qt::UserRole + 1;

	IPFilterDialog::IPFilterDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		connect (Ui_.Tree_,
				&QTreeWidget::currentItemChanged,
				[this] (QTreeWidgetItem *item)
				{
					Ui_.Modify_->setEnabled (item);
					Ui_.Remove_->setEnabled (item);
				});
		connect (Ui_.Tree_,
				&QTreeWidget::itemClicked,
				[] (QTreeWidgetItem *item, int column)
				{
					if (column != 2)
						return;

					bool block = !item->data (2, BlockRole).toBool ();
					item->setData (2, BlockRole, block);
					item->setText (2, block ?
							tr ("block") :
							tr ("allow"));
				});

		connect (Ui_.Add_,
				&QPushButton::released,
				this,
				&IPFilterDialog::Add);
		connect (Ui_.Modify_,
				&QPushButton::released,
				this,
				&IPFilterDialog::Modify);
		connect (Ui_.Remove_,
				&QPushButton::released,
				[this] { delete Ui_.Tree_->currentItem (); });

		for (const auto& [key, block] : Util::Stlize (Core::Instance ()->GetFilter ()))
		{
			const auto item = new QTreeWidgetItem (Ui_.Tree_);
			item->setText (0, key.first);
			item->setText (1, key.second);
			item->setText (2, block ?
					tr ("block") :
					tr ("allow"));
			item->setData (2, BlockRole, block);
		}
	}

	QList<QPair<BanRange_t, bool>> IPFilterDialog::GetFilter () const
	{
		QList<QPair<BanRange_t, bool>> result;
		for (int i = 0, size = Ui_.Tree_->topLevelItemCount (); i < size; ++i)
		{
			QTreeWidgetItem *item = Ui_.Tree_->topLevelItem (i);
			result << qMakePair (qMakePair (item->text (0), item->text (1)),
					item->data (2, BlockRole).toBool ());
		}
		return result;
	}

	void IPFilterDialog::Add ()
	{
		BanPeersDialog dia;
		if (dia.exec () != QDialog::Accepted)
			return;

		const auto& start = dia.GetStart ();
		const auto& end = dia.GetEnd ();
		if (start.isEmpty () ||
				end.isEmpty ())
			return;

		const auto item = new QTreeWidgetItem (Ui_.Tree_);
		item->setText (0, start);
		item->setText (1, end);
		item->setText (2, tr ("block"));
		item->setData (2, BlockRole, true);
	}

	void IPFilterDialog::Modify ()
	{
		BanPeersDialog dia;
		const auto item = Ui_.Tree_->currentItem ();
		dia.SetIP (item->text (0), item->text (1));
		if (dia.exec () != QDialog::Accepted)
			return;

		const auto& start = dia.GetStart ();
		const auto& end = dia.GetEnd ();
		if (start.isEmpty () ||
				end.isEmpty ())
			return;

		item->setText (0, start);
		item->setText (1, end);
	}
}
