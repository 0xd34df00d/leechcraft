/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_ljaccountconfigurationdialog.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	class LJAccountConfigurationWidget;

	class LJAccountConfigurationDialog : public QDialog
	{
		Ui::LJAccountConfigurationDialog Ui_;
	public:
		LJAccountConfigurationDialog (QWidget *parent = 0);
		LJAccountConfigurationWidget* ConfWidget ();
	};
}
}
}
