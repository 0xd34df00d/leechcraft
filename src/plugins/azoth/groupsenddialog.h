/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_groupsenddialog.h"

class QStandardItemModel;
class QStandardItem;

namespace LC
{
namespace Azoth
{
	class ICLEntry;

	class GroupSendDialog : public QDialog
	{
		Q_OBJECT

		Ui::GroupSendDialog Ui_;
		QStandardItemModel *ContactsModel_;
		QMap<ICLEntry*, QStandardItem*> Entry2Item_;
	public:
		GroupSendDialog (const QList<ICLEntry*>&, QWidget* = 0);
	private slots:
		void on_Message__textChanged ();
		void on_SendButton__released ();
		void on_AllButton__released ();
		void on_NoneButton__released ();
		void on_OnlineButton__released ();
		void on_OfflineButton__released ();
		void handleEntryStatusChanged ();
		void handleEntryDestroyed ();
	};
}
}
