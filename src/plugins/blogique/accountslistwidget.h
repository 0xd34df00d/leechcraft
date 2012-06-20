/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QHash>
#include "ui_accountslistwidget.h"

class QStandardItemModel;
class QStandardItem;

namespace LeechCraft
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
	};
}
}
