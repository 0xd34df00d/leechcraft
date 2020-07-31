/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_accountconfigdialog.h"

namespace LC
{
namespace Azoth
{
namespace VelvetBird
{
	class AccountConfigDialog : public QDialog
	{
		Ui::AccountConfigDialog Ui_;
	public:
		AccountConfigDialog (QWidget* = 0);

		QString GetUser () const;
		void SetUser (const QString&);

		QString GetNick () const;
		void SetNick (const QString&);
	};
}
}
}
