/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_vcardlisteditdialog.h"

class QStandardItemModel;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class VCardListEditDialog : public QDialog
	{
		Q_OBJECT

		Ui::VCardListEditDialog Ui_;

		QStandardItemModel *Model_;
	public:
		VCardListEditDialog (const QStringList& options, QWidget* = 0);

		void AddItems (const QList<QPair<QString, QStringList>>&);
		QList<QPair<QString, QStringList>> GetItems () const;
	private slots:
		void on_Add__released ();
		void on_Remove__released ();
	};
}
}
}
