/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCACCOUNTCONFIGURATIONDIALOG_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCACCOUNTCONFIGURATIONDIALOG_H

#include <QDialog>
#include "ui_ircaccountconfigurationdialog.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{

	class IrcAccountConfigurationDialog : public QDialog
	{
		Ui::IrcAccountConfigurationDialog Ui_;
	public:
		IrcAccountConfigurationDialog (QWidget* = 0);

		IrcAccountConfigurationWidget* ConfWidget () const;
	};
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCACCOUNTCONFIGURATIONDIALOG_H
