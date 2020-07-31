/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_accountpropsdialog.h"

namespace LC
{
namespace Poleemery
{
	struct Account;

	class AccountPropsDialog : public QDialog
	{
		Q_OBJECT

		Ui::AccountPropsDialog Ui_;
		int CurrentAccID_;
	public:
		AccountPropsDialog (QWidget* = 0);

		void SetAccount (const Account&);
		Account GetAccount () const;
	};
}
}
