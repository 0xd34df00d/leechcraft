/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_NETSTOREMANAGER_ACCOUNTSLISTWIDGET_H
#define PLUGINS_NETSTOREMANAGER_ACCOUNTSLISTWIDGET_H
#include <QWidget>
#include "ui_accountslistwidget.h"

namespace LC
{
namespace NetStoreManager
{
	class AccountsManager;

	class AccountsListWidget : public QWidget
	{
		Q_OBJECT

		Ui::AccountsListWidget Ui_;
		AccountsManager *Manager_;
	public:
		AccountsListWidget (AccountsManager*, QWidget* = 0);
	private slots:
		void on_Add__released ();
		void on_Remove__released ();
	};
}
}

#endif
