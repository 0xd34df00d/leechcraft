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

#include <QItemDelegate>

class QComboBox;

namespace LeechCraft
{
namespace NetStoreManager
{
	class AccountsManager;

	class SyncItemDelegate : public QItemDelegate
	{
		Q_OBJECT

		AccountsManager *AM_;

	public:
		enum Columns
		{
			Account,
			Directory
		};

		enum SyncItemDelegateRoles
		{
			AccountId = Qt::UserRole + 1
		};

		SyncItemDelegate (AccountsManager *am, QObject *parent = 0);

		QWidget* createEditor (QWidget *parent,
				const QStyleOptionViewItem& option, const QModelIndex& index) const;
		void setEditorData (QWidget *editor, const QModelIndex& index) const;
		void setModelData (QWidget *editor, QAbstractItemModel *model,
				const QModelIndex& index) const;
		void updateEditorGeometry (QWidget *editor,
				const QStyleOptionViewItem& option, const QModelIndex& index) const;
	private:
		void FillAccounts (QComboBox *box) const;
// 		void FillAccountsAndSetCurrent (QComboBox *box, );

	private slots:
		void handleCloseDirectoryEditor (QWidget *w);
	};
}
}
