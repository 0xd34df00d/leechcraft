/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "addfrommisseddialog.h"

namespace LC
{
namespace AdvancedNotifications
{
	AddFromMissedDialog::AddFromMissedDialog (QAbstractItemModel *model, QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);
		Ui_.MissedView_->setModel (model);
	}

	QList<QModelIndex> AddFromMissedDialog::GetSelectedRows () const
	{
		return Ui_.MissedView_->selectionModel ()->selectedRows ();
	}
}
}
