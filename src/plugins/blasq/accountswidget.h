/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_accountswidget.h"

namespace LC
{
namespace Blasq
{
	class ServicesManager;
	class AccountsManager;

	class AccountsWidget : public QWidget
	{
		Q_OBJECT

		Ui::AccountsWidget Ui_;

		ServicesManager * const ServicesMgr_;
		AccountsManager * const AccountsMgr_;
	public:
		AccountsWidget (ServicesManager*, AccountsManager*, QWidget* = 0);
	private slots:
		void on_Add__released ();
		void on_Remove__released ();
	};
}
}
