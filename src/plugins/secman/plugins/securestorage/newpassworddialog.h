/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Alexander Konovalov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_newpassworddialog.h"

namespace LC
{
namespace SecMan
{
namespace SecureStorage
{
	class NewPasswordDialog : public QDialog
	{
		Q_OBJECT

		Ui::NewPasswordDialog Ui_;
	public:
		NewPasswordDialog ();
		QString GetPassword () const;
	public slots:
		void clear ();
	private slots:
		void checkPasswords ();
	signals:
		void dialogFinished ();
	};
}
}
}
