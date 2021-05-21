/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_addfrommisseddialog.h"

namespace LC::AdvancedNotifications
{
	class AddFromMissedDialog : public QDialog
	{
		Ui::AddFromMissedDialog Ui_;
	public:
		explicit AddFromMissedDialog (QAbstractItemModel*, QWidget* = nullptr);

		QList<QModelIndex> GetSelectedRows () const;
	};
}
