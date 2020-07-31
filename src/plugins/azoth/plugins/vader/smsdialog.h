/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_smsdialog.h"

namespace LC
{
namespace Azoth
{
namespace Vader
{
	class SMSDialog : public QDialog
	{
		Q_OBJECT

		Ui::SMSDialog Ui_;
	public:
		SMSDialog (QString, QWidget* = 0);

		QString GetPhone () const;
		QString GetText () const;
	private slots:
		void on_Text__textChanged ();
	};
}
}
}
