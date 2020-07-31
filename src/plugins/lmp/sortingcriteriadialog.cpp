/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sortingcriteriadialog.h"
#include <QInputDialog>
#include <QStandardItemModel>

namespace LC
{
namespace LMP
{
	namespace
	{
		enum Roles
		{
			CriteriaRole = Qt::UserRole + 1
		};
	}

	SortingCriteriaDialog::SortingCriteriaDialog (QWidget *parent)
	: QDialog (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.CriteriaView_->setModel (Model_);
	}

	void SortingCriteriaDialog::SetCriteria (const QList<SortingCriteria>& criteria)
	{
		for (auto crit : criteria)
			AddCriteria (crit);
	}

	QList<SortingCriteria> SortingCriteriaDialog::GetCriteria () const
	{
		QList<SortingCriteria> result;
		for (int i = 0; i < Model_->rowCount (); ++i)
			result << static_cast<SortingCriteria> (Model_->item (i)->data (CriteriaRole).toInt ());
		return result;
	}

	void SortingCriteriaDialog::AddCriteria (SortingCriteria criteria)
	{
		auto item = new QStandardItem (GetCriteriaName (criteria));
		item->setData (static_cast<int> (criteria), CriteriaRole);
		Model_->appendRow (item);
	}

	void SortingCriteriaDialog::on_Add__released ()
	{
		const auto& existing = GetCriteria ();
		auto criteria = GetAllCriteria ();
		for (auto i = criteria.begin (); i != criteria.end (); )
			if (existing.contains (*i))
				i = criteria.erase (i);
			else
				++i;

		QStringList items;
		for (const auto& item : criteria)
			items << GetCriteriaName (item);

		bool ok = false;
		const auto& selected = QInputDialog::getItem (this,
				tr ("Select criteria"),
				tr ("Select criteria to be added:"),
				items,
				0,
				false,
				&ok);
		if (!ok)
			return;

		const auto& pos = items.indexOf (selected);
		if (pos < 0)
			return;

		AddCriteria (criteria.at (pos));
	}

	void SortingCriteriaDialog::on_Remove__released ()
	{
		const auto& current = Ui_.CriteriaView_->currentIndex ();
		if (!current.isValid ())
			return;

		Model_->removeRow (current.row ());
	}

	void SortingCriteriaDialog::on_MoveUp__released ()
	{
		const auto& current = Ui_.CriteriaView_->currentIndex ();
		const int row = current.row ();
		if (row <= 0)
			return;

		Model_->insertRow (row - 1, Model_->takeRow (row));
		const auto& newIdx = current.sibling (current.row () - 1, 0);
		Ui_.CriteriaView_->selectionModel ()->select (newIdx, QItemSelectionModel::ClearAndSelect);
		Ui_.CriteriaView_->setCurrentIndex (newIdx);
	}

	void SortingCriteriaDialog::on_MoveDown__released ()
	{
		const auto& current = Ui_.CriteriaView_->currentIndex ();
		if (!current.isValid ())
			return;

		const int row = current.row ();
		if (row >= Model_->rowCount () - 1)
			return;

		Model_->insertRow (row + 1, Model_->takeRow (row));
		const auto& newIdx = current.sibling (current.row () + 1, 0);
		Ui_.CriteriaView_->selectionModel ()->select (newIdx, QItemSelectionModel::ClearAndSelect);
		Ui_.CriteriaView_->setCurrentIndex (newIdx);
	}
}
}
