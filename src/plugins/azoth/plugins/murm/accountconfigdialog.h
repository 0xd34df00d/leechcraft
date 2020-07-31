/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_accountconfigdialog.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	class AccountConfigDialog : public QDialog
	{
		Q_OBJECT

		Ui::AccountConfigDialog Ui_;
	public:
		AccountConfigDialog (QWidget* = 0);

		bool GetFileLogEnabled () const;
		void SetFileLogEnabled (bool);

		bool GetUpdateStatusEnabled () const;
		void SetUpdateStatusEnabled (bool);

		bool GetPublishTuneEnabled () const;
		void SetPublishTuneEnabled (bool);

		bool GetMarkAsOnline () const;
		void SetMarkAsOnline (bool);
	signals:
		void reauthRequested ();
	};
}
}
}
