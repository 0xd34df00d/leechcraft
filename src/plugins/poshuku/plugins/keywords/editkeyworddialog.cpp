/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "editkeyworddialog.h"

namespace LC
{
namespace Poshuku
{
namespace Keywords
{ 
	EditKeywordDialog::EditKeywordDialog (const QString& url, 
			const QString& keyword, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		Ui_.Url_->setText (url);
		Ui_.Keyword_->setText (keyword);
		Ui_.Url_->setFocus (Qt::ActiveWindowFocusReason);
	}

	QString EditKeywordDialog::GetUrl () const
	{
		return Ui_.Url_->text ();
	}

	QString EditKeywordDialog::GetKeyword () const
	{
		return Ui_.Keyword_->text ();
	}
}
}
}

