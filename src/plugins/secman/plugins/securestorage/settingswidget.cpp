/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Alexander Konovalov
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "settingswidget.h"
#include "securestorage.h"

namespace LeechCraft
{
namespace Plugins
{
namespace SecMan
{
namespace StoragePlugins
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
}
}
