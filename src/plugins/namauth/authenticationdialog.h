/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "ui_authenticationdialog.h"

namespace LC::NamAuth
{
	class AuthenticationDialog : public QDialog
	{
		Ui::AuthenticationDialog Ui_;
	public:
		AuthenticationDialog (const QString& message,
				const QString& login,
				const QString& password,
				QWidget *parent = nullptr);

		QString GetLogin () const;
		QString GetPassword () const;
		bool ShouldSave () const;
	};
}
