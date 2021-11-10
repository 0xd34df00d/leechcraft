/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_ircaccountconfigurationdialog.h"

namespace LC::Azoth::Acetamide
{
	class IrcAccountConfigurationDialog : public QDialog
	{
		Ui::IrcAccountConfigurationDialog Ui_;
	public:
		explicit IrcAccountConfigurationDialog (QWidget* = nullptr);

		IrcAccountConfigurationWidget* ConfWidget () const;
	};
}
