/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_showtoxiddialog.h"

namespace LC::Azoth::Sarin
{
	class ShowToxIdDialog : public QDialog
	{
		Q_OBJECT

		Ui::ShowToxIdDialog Ui_;
	public:
		ShowToxIdDialog (const QString& placeholder, QWidget* = nullptr);
	public slots:
		void setToxId (const QString&);
	private slots:
		void on_CopyButton__released ();
	};
}
