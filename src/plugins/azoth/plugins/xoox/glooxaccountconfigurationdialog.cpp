/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "glooxaccountconfigurationdialog.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	GlooxAccountConfigurationDialog::GlooxAccountConfigurationDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
	}
	
	GlooxAccountConfigurationWidget* GlooxAccountConfigurationDialog::W () const
	{
		return Ui_.ConfWidget_;
	}
}
}
}
