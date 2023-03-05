/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "closedialog.h"

namespace LC::Eleeminator
{
	CloseDialog::CloseDialog (QAbstractItemModel *model, QWidget *parent)
	: QDialog { parent }
	, Model_ { model }
	{
		Ui_.setupUi (this);
		Ui_.ChildView_->setModel (model);
		Ui_.ChildView_->expandAll ();
	}
}
