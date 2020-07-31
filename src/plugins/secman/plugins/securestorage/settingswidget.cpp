/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Alexander Konovalov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "settingswidget.h"
#include "securestorage.h"

namespace LC
{
namespace SecMan
{
namespace SecureStorage
{
	SettingsWidget::SettingsWidget ()
	{
		Ui_.setupUi (this);
		connect (Ui_.ChangePassword_,
				SIGNAL (clicked ()),
				this,
				SIGNAL (changePasswordRequested ()));
		connect (Ui_.ClearSettings_,
				SIGNAL (clicked ()),
				this,
				SIGNAL (clearSettingsRequested ()));
	}

	QString SettingsWidget::GetOldPassword ()
	{
		return Ui_.OldPassword_->text ();
	}

	QString SettingsWidget::GetNewPassword ()
	{
		return ReturnIfEqual (Ui_.NewPassword1_->text (), Ui_.NewPassword2_->text ());
	}

	void SettingsWidget::ClearPasswordFields ()
	{
		Ui_.OldPassword_->clear ();
		Ui_.NewPassword1_->clear ();
		Ui_.NewPassword2_->clear ();
	}
}
}
}
