/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ircaccountconfigurationdialog.h"

namespace LC::Azoth::Acetamide
{
	IrcAccountConfigurationDialog::IrcAccountConfigurationDialog (QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);
	}

	IrcAccountConfigurationWidget* IrcAccountConfigurationDialog::ConfWidget () const
	{
		return Ui_.ConfWidget_;
	}
}
