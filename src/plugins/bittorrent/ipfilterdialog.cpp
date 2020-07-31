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

namespace LC
{
namespace BitTorrent
{
	const int BlockRole = 101;

	IPFilterDialog::IPFilterDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		const auto& filter = Core::Instance ()->GetFilter ();
		for (const auto& pair : Util::Stlize (filter))
		{
			const auto& key = pair.first;

			const auto item = new QTreeWidgetItem (Ui_.Tree_);
			item->setText (0, key.first);
			item->setText (1, key.second);
			bool block = pair.second;
			item->setText (2, block ?
					tr ("block") :
					tr ("allow"));
			item->setData (2, BlockRole, block);
		}

		on_Tree__currentItemChanged (0);
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

	void IPFilterDialog::on_Tree__currentItemChanged (QTreeWidgetItem *current)
	{
		Ui_.Modify_->setEnabled (current);
		Ui_.Remove_->setEnabled (current);
	}

	void IPFilterDialog::on_Tree__itemClicked (QTreeWidgetItem *item, int column)
	{
		if (column != 2)
			return;

		bool block = !item->data (2, BlockRole).toBool ();
		item->setData (2, BlockRole, block);
		item->setText (2, block ?
				tr ("block") :
				tr ("allow"));
	}

	void IPFilterDialog::on_Add__released ()
	{
		BanPeersDialog dia;
		if (dia.exec () != QDialog::Accepted)
			return;

		QString start = dia.GetStart ();
		QString end = dia.GetEnd ();
		if (start.isEmpty () ||
				end.isEmpty ())
			return;

		QTreeWidgetItem *item = new QTreeWidgetItem (Ui_.Tree_);
		item->setText (0, start);
		item->setText (1, end);
		item->setText (2, tr ("block"));
		item->setData (2, BlockRole, true);
	}

	void IPFilterDialog::on_Modify__released ()
	{
		BanPeersDialog dia;
		QTreeWidgetItem *item = Ui_.Tree_->currentItem ();
		dia.SetIP (item->text (0), item->text (1));
		if (dia.exec () != QDialog::Accepted)
			return;

		QString start = dia.GetStart ();
		QString end = dia.GetEnd ();
		if (start.isEmpty () ||
				end.isEmpty ())
			return;

		item->setText (0, start);
		item->setText (1, end);
	}

	void IPFilterDialog::on_Remove__released ()
	{
		delete Ui_.Tree_->currentItem ();
	}
}
}
