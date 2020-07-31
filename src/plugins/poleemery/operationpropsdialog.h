/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_operationpropsdialog.h"
#include "structures.h"

namespace LC
{
namespace Poleemery
{
	class EntriesModel;
	enum class EntryType;
	struct ExpenseEntry;
	struct ReceiptEntry;

	class OperationPropsDialog : public QDialog
	{
		Q_OBJECT

		Ui::OperationPropsDialog Ui_;
		const QList<Account> Accounts_;

		QStringList ExpenseNames_;
		QStringList ReceiptNames_;
		QStringList ShopNames_;

		EntriesModel *ItemsModel_;
	public:
		OperationPropsDialog (QWidget* = 0);

		EntryType GetEntryType () const;

		QList<EntryBase_ptr> GetEntries () const;
	private:
		QDateTime GetDateTime () const;
	private slots:
		void on_AccsBox__currentIndexChanged (int);
		void on_AmountCurrency__currentIndexChanged (int);
		void on_DateEdit__dateTimeChanged (const QDateTime&);
		void on_Shop__editTextChanged (const QString&);

		void on_ExpenseEntry__released ();
		void on_ReceiptEntry__released ();

		void on_AddEntry__released ();
		void on_RemoveEntry__released ();

		void recalcNatives ();
	};
}
}
