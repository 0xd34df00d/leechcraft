/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accregfirstpage.h"

namespace LC
{
namespace Azoth
{
namespace VelvetBird
{
	AccRegFirstPage::AccRegFirstPage (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
	}

	QString AccRegFirstPage::GetName () const
	{
		return Ui_.Name_->text ();
	}

	QString AccRegFirstPage::GetNick () const
	{
		return Ui_.Nick_->text ();
	}
}
}
}
