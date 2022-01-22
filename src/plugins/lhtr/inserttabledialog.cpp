/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "inserttabledialog.h"

namespace LC::LHTR
{
	InsertTableDialog::InsertTableDialog (QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);
	}

	QString InsertTableDialog::GetCaption () const
	{
		return Ui_.Caption_->text ();
	}

	int InsertTableDialog::GetColumns () const
	{
		return Ui_.Columns_->value ();
	}

	int InsertTableDialog::GetRows () const
	{
		return Ui_.Rows_->value ();
	}
}
