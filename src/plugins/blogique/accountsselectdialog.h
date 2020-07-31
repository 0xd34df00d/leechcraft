/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_accountsselectdialog.h"

class QStandardItem;
class QStandardItemModel;

namespace LC
{
namespace Blogique
{
	class IAccount;

	class AccountsSelectDialog : public QDialog
	{
		Q_OBJECT

		Ui::AccountsSelectDialog Ui_;

		QStandardItemModel *Model_;
		QHash<QStandardItem*, IAccount*> Item2Accotun_;

	public:
		AccountsSelectDialog (QWidget *parent = 0);

		void FillAccounts (const QList<IAccount*>& accounts);
		QList<IAccount*> GetSelectedAccounts () const;
	};
}
}

