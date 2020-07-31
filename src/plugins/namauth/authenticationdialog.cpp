/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "authenticationdialog.h"

namespace LC
{
namespace NamAuth
{
	AuthenticationDialog::AuthenticationDialog (const QString& message,
			const QString& login,
			const QString& password,
			QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);
		Ui_.Message_->setText (message);
		Ui_.LoginEdit_->setText (login);
		Ui_.PasswordEdit_->setText (password);
	}

	QString AuthenticationDialog::GetLogin () const
	{
		return Ui_.LoginEdit_->text ();
	}

	QString AuthenticationDialog::GetPassword () const
	{
		return Ui_.PasswordEdit_->text ();
	}

	bool AuthenticationDialog::ShouldSave () const
	{
		return Ui_.SaveCredentials_->checkState () == Qt::Checked;
	}
}
}
