/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "authenticationdialog.h"

using namespace LeechCraft;

LeechCraft::AuthenticationDialog::AuthenticationDialog (const QString& message,
		const QString& login,
		const QString& password,
		QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);
	Ui_.Message_->setText (message);
	Ui_.LoginEdit_->setText (login);
	Ui_.PasswordEdit_->setText (password);
}

QString LeechCraft::AuthenticationDialog::GetLogin () const
{
	return Ui_.LoginEdit_->text ();
}

QString LeechCraft::AuthenticationDialog::GetPassword () const
{
	return Ui_.PasswordEdit_->text ();
}

bool LeechCraft::AuthenticationDialog::ShouldSave () const
{
	return Ui_.SaveCredentials_->checkState () == Qt::Checked;
}

