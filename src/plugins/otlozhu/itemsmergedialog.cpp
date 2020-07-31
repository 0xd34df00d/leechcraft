/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemsmergedialog.h"

namespace LC
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
