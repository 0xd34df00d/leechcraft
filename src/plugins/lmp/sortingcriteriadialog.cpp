/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "sortingcriteriadialog.h"
#include <QInputDialog>
#include <QStandardItemModel>

namespace LeechCraft
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
		const auto& criteria = GetAllCriteria ();
		QStringList items;
		for (const auto& item : criteria)
			items << GetCriteriaName (item);

		const auto& selected = QInputDialog::getItem (this,
				tr ("Select criteria"),
				tr ("Select criteria to be added:"),
				items,
				0,
				false);
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
}
}
