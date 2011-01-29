/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef AUTHENTICATIONDIALOG_H
#define AUTHENTICATIONDIALOG_H
#include <QDialog>
#include "ui_authenticationdialog.h"

namespace LeechCraft
{
	/** Provides a standard authentication dialog, for example, for
	 * proxies, SSL stuff etc.
	 */
	class AuthenticationDialog : public QDialog
	{
		Q_OBJECT

		Ui::AuthenticationDialog Ui_;
	public:
		/** Initializes the dialog. Sets initial login to login, initial
		 * password to password and message of the dialog to message.
		 *
		 * @param[in] message The message explaining the dialog.
		 * @param[in] login Initial (suggested) login.
		 * @param[in] password Initial (suggested) password.
		 * @param[in] parent Parent widget of this dialog.
		 */
		AuthenticationDialog (const QString& message,
				const QString& login,
				const QString& password,
				QWidget *parent = 0);

		/** Returns the login.
		 *
		 * @return The login.
		 */
		QString GetLogin () const;

		/** Returns the password.
		 *
		 * @return The password.
		 */
		QString GetPassword () const;

		/** Returns whether user has chosen to save authentication data.
		 *
		 * @return True if auth data should be saved, false otherwise.
		 */
		bool ShouldSave () const;
	};
};

#endif

