/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
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
namespace Azoth
{
	class IAccount;

	class AccountsListWidget : public QWidget
	{
		Q_OBJECT

		Ui::AccountsListWidget Ui_;
		QStandardItemModel * const AccModel_;
		QHash<IAccount*, QStandardItem*> Account2Item_;
	public:
		enum Role
		{
			AccObj = Qt::UserRole + 1,
			ChatStyleManager,
			MUCStyleManager
		};

		enum Column
		{
			ShowInRoster,
			Name,
			ChatStyle,
			ChatVariant,
			MUCStyle,
			MUCVariant
		};

		AccountsListWidget (QWidget* = 0);
	private slots:
		void addAccount (IAccount*);

		void on_Add__released ();

		void on_Modify__released ();
		void on_PGP__released ();
		void on_Delete__released ();
		void on_ResetStyles__released ();

		void handleAccountSelected (const QModelIndex&);

		void handleItemChanged (QStandardItem*);

		void handleAccountRemoved (IAccount*);
	signals:
		void accountVisibilityChanged (IAccount*);
	};
}
}
