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

#include "itemsmergedialog.h"

namespace LeechCraft
{
namespace Otlozhu
{
	ItemsMergeDialog::ItemsMergeDialog (int size, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		Ui_.TextLabel_->setText (tr ("There are %n items to import. "
					"How would you like to merge similar tasks?", 0, size));
	}

	ItemsMergeDialog::Priority ItemsMergeDialog::GetPriority () const
	{
		return Ui_.PrioImport_->isChecked () ?
				Priority::Imported :
				Priority::Current;
	}

	ItemsMergeDialog::SameTitle ItemsMergeDialog::GetSameTitle () const
	{
		return Ui_.TitleMerge_->isChecked () ?
				SameTitle::Merge :
				SameTitle::LeaveDistinct;
	}
}
}
