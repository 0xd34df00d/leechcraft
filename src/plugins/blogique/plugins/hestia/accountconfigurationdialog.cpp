/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountconfigurationdialog.h"

namespace LC
{
namespace Blogique
{
namespace Hestia
{
	AccountConfigurationDialog::AccountConfigurationDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
	}

	AccountConfigurationWidget* AccountConfigurationDialog::ConfWidget ()
	{
		return Ui_.ConfWidget_;
	}

}
}
}


