/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_accountconfigurationdialog.h"

namespace LC
{
namespace Blogique
{
namespace Hestia
{
	class AccountConfigurationWidget;

	class AccountConfigurationDialog : public QDialog
	{
		Ui::AccountConfigurationDialog Ui_;
	public:
		AccountConfigurationDialog (QWidget *parent = 0);
		AccountConfigurationWidget* ConfWidget ();
	};
}
}
}
