/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "editcommentdialog.h"

namespace LC
{
namespace Otlozhu
{
	EditCommentDialog::EditCommentDialog (const QString& title, const QString& comment, QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);

		Ui_.TaskNameLabel_->setText (title);
		Ui_.EditField_->setText (comment);
	}

	QString EditCommentDialog::GetComment () const
	{
		return Ui_.EditField_->toHtml ();
	}
}
}
