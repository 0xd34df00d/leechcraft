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

#include "newpassworddialog.h"
#include <QString>
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
	NewPasswordDialog::NewPasswordDialog ()
	{
		Ui_.setupUi (this);

		connect (Ui_.PasswordEdit1_,
			SIGNAL (textChanged (const QString&)),
			this,
			SLOT (checkPasswords ()));
		connect (Ui_.PasswordEdit2_,
			SIGNAL (textChanged (const QString&)),
			this,
			SLOT (checkPasswords ()));

		connect (this,
			SIGNAL (accepted ()),
			this,
			SIGNAL (dialogFinished ()),
			Qt::QueuedConnection);
		connect (this,
			SIGNAL (rejected ()),
			this,
			SIGNAL (dialogFinished ()),
			Qt::QueuedConnection);
	}

	QString NewPasswordDialog::GetPassword () const
	{
		return ReturnIfEqual (Ui_.PasswordEdit1_->text (),  Ui_.PasswordEdit2_->text ());
	}

	void NewPasswordDialog::clear ()
	{
		Ui_.PasswordEdit1_->clear ();
		Ui_.PasswordEdit2_->clear ();
	}

	void NewPasswordDialog::checkPasswords ()
	{
		bool passwordsEqual = Ui_.PasswordEdit1_->text () == Ui_.PasswordEdit2_->text ();
		Ui_.DifferenceLabel_->setText (passwordsEqual ? "" : tr ("Passwords must be the same"));
		Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (passwordsEqual);
	}

}
}
}
}
}
