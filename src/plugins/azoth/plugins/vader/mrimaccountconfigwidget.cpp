/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mrimaccountconfigwidget.h"

namespace LC
{
namespace Azoth
{
namespace Vader
{
	MRIMAccountConfigWidget::MRIMAccountConfigWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
	}

	QString MRIMAccountConfigWidget::GetLogin () const
	{
		return Ui_.Email_->text ();
	}

	QString MRIMAccountConfigWidget::GetPassword () const
	{
		return Ui_.Password_->text ();
	}
}
}
}
