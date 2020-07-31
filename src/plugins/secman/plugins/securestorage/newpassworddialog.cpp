/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Alexander Konovalov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "newpassworddialog.h"
#include <QString>
#include "securestorage.h"

namespace LC
{
namespace SecMan
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
