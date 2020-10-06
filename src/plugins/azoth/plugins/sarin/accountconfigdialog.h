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

namespace LC::Azoth::Sarin
{
	struct ToxAccountConfiguration;

	class AccountConfigDialog : public QDialog
	{
		Ui::AccountConfigDialog Ui_;
	public:
		explicit AccountConfigDialog (QDialog* = nullptr);

		ToxAccountConfiguration GetConfig () const;
		void SetConfig (const ToxAccountConfiguration&);
	};
}
