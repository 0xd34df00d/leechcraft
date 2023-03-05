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
	CloseDialog::CloseDialog (std::unique_ptr<QAbstractItemModel> model, QWidget *parent)
	: QDialog { parent }
	, Model_ { std::move (model) }
	{
		Ui_.setupUi (this);
		Ui_.ChildView_->setModel (Model_.get ());
		Ui_.ChildView_->expandAll ();
	}
}
