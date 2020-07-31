/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_submittodialog.h"

class QStandardItem;
class QStandardItemModel;

namespace LC
{
namespace Blogique
{
	class IAccount;

	class SubmitToDialog : public QDialog
	{
		Q_OBJECT

		Ui::SubmitToDialog Ui_;
		QStandardItemModel *Model_;
		QHash<QStandardItem*, IAccount*> Item2Account_;

	public:
		enum Columns
		{
			Account,
			Target
		};

		explicit SubmitToDialog (QWidget *parent = 0);

		IAccount* GetAccountFromIndex (const QModelIndex& index) const;
		QAbstractItemModel* GetModel () const;

		QList<QPair<IAccount*, QString>> GetPostingTargets () const;
	};
}
}
