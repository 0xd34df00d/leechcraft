/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "simpledialog.h"

namespace LC
{
namespace Azoth
{
	SimpleDialog::SimpleDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
	}
	
	void SimpleDialog::SetWidget (QWidget *w)
	{
		Ui_.Layout_->insertWidget (0, w);
		connect (this,
				SIGNAL (accepted ()),
				w,
				SLOT (accept ()));
	}
}
}
