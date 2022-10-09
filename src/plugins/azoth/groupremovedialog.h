/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_groupremovedialog.h"

class QStandardItemModel;

namespace LC
{
namespace Azoth
{
	class ICLEntry;

	class GroupRemoveDialog : public QDialog
	{
		Q_OBJECT

		Ui::GroupRemoveDialog Ui_;

		const QList<ICLEntry*> Entries_;
		QStandardItemModel *Model_;
	public:
		GroupRemoveDialog (const QList<ICLEntry*>&, QWidget* = 0);

		QList<ICLEntry*> GetSelectedEntries () const;

		void accept ();
	};
}
}
