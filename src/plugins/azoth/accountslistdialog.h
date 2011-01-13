/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_ACCOUNTSLISTDIALOG_H
#define PLUGINS_AZOTH_ACCOUNTSLISTDIALOG_H
#include <QDialog>
#include <QHash>
#include "ui_accountslistdialog.h"

class QStandardItemModel;
class QStandardItem;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			namespace Plugins
			{
				class IAccount;
			}

			class AccountsListDialog : public QDialog
			{
				Q_OBJECT

				Ui::AccountsListDialog Ui_;
				QStandardItemModel *AccModel_;
				QHash<Plugins::IAccount*, QStandardItem*> Account2Item_;

				enum Roles
				{
					RAccObj = Qt::UserRole + 1
				};
			public:
				AccountsListDialog (QWidget* = 0);
			private slots:
				void addAccount (Plugins::IAccount*);
				void on_Modify__released ();
				void on_Delete__released ();

				void handleAccountRemoved (Plugins::IAccount*);
			};
		}
	}
}

#endif
