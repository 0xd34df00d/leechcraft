/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
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

#include <QDialog>
#include "ui_submittodialog.h"

class QStandardItem;
class QStandardItemModel;

namespace LeechCraft
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
