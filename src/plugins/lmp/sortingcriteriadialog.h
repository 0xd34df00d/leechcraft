/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_sortingcriteriadialog.h"
#include "sortingcriteria.h"

class QStandardItemModel;

namespace LC
{
namespace LMP
{
	class SortingCriteriaDialog : public QDialog
	{
		Q_OBJECT

		Ui::SortingCriteriaDialog Ui_;
		QStandardItemModel *Model_;
	public:
		SortingCriteriaDialog (QWidget* = 0);

		void SetCriteria (const QList<SortingCriteria>&);
		QList<SortingCriteria> GetCriteria () const;
	private:
		void AddCriteria (SortingCriteria);
	private slots:
		void on_Add__released ();
		void on_Remove__released ();

		void on_MoveUp__released ();
		void on_MoveDown__released ();
	};
}
}
