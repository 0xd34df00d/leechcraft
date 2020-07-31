/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QItemDelegate>

class QStandardItemModel;
class QComboBox;

namespace LC
{
namespace NetStoreManager
{
	class AccountsManager;

	class SyncItemDelegate : public QItemDelegate
	{
		Q_OBJECT

		AccountsManager *AM_;
		QStandardItemModel *Model_;

	public:
		enum Columns
		{
			Account,
			LocalDirectory,
			RemoteDirectory
		};

		enum SyncItemDelegateRoles
		{
			AccountId = Qt::UserRole + 1
		};

		SyncItemDelegate (AccountsManager *am, QStandardItemModel *model, QObject *parent = 0);

		QWidget* createEditor (QWidget *parent,
				const QStyleOptionViewItem& option, const QModelIndex& index) const;
		void setEditorData (QWidget *editor, const QModelIndex& index) const;
		void setModelData (QWidget *editor, QAbstractItemModel *model,
				const QModelIndex& index) const;
		void updateEditorGeometry (QWidget *editor,
				const QStyleOptionViewItem& option, const QModelIndex& index) const;
	private:
		void FillAccounts (QComboBox *box) const;

	private slots:
		void handleCloseDirectoryEditor (QWidget *w);
	};
}
}
