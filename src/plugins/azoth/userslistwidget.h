/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QDialog>
#include "ui_userslistwidget.h"

class QStandardItemModel;
class QSortFilterProxyModel;

namespace LC
{
namespace Azoth
{
	class ICLEntry;

	class UsersListWidget : public QDialog
	{
		Q_OBJECT

		Ui::UsersListWidget Ui_;

		QSortFilterProxyModel *Filter_;
		QStandardItemModel *PartsModel_;
	public:
		UsersListWidget (const QList<QObject*>&, std::function<QString (ICLEntry*)>, QWidget* = 0);

		QObject* GetActivatedParticipant () const;
	};
}
}
