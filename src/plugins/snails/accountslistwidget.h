/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_accountslistwidget.h"

namespace LC
{
namespace Snails
{
	class AccountsManager;

	class AccountsListWidget : public QWidget
	{
		Q_OBJECT

		Ui::AccountsListWidget Ui_;
		AccountsManager * const AccsMgr_;
	public:
		AccountsListWidget (AccountsManager*, QWidget* = 0);
	private slots:
		void on_AddButton__released ();
		void on_ModifyButton__released ();
		void on_RemoveButton__released ();
	};
}
}
