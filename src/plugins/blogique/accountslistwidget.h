/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QHash>
#include "ui_accountslistwidget.h"

class QStandardItemModel;
class QStandardItem;

namespace LC
{
namespace Blogique
{
	class IAccount;

	class AccountsListWidget : public QWidget
	{
		Q_OBJECT

		enum Columns
		{
			Name,
			IsValidated
		};

		Ui::AccountsListWidget Ui_;
		QStandardItemModel *AccountsModel_;
		QHash<QStandardItem*, IAccount*> Item2Account_;
		QHash<IAccount*, QStandardItem*> Account2Item_;
	public:
		AccountsListWidget (QWidget* = 0);

	public slots:
		void addAccount (QObject *accObj);
		void handleAccountRemoved (QObject *accObj);
		void handleAccountValidated (QObject *accObj, bool validated);
	private slots:
		void on_Add__released ();
		void on_Modify__released ();
		void on_Delete__released ();
		void on_Profile__released ();
		void handleAccountClicked (const QModelIndex& index);
		void handleAccountDoubleClicked (const QModelIndex& index);
	};
}
}
