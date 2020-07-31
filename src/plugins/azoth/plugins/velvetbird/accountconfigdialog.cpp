/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountconfigdialog.h"

namespace LC
{
namespace Azoth
{
namespace VelvetBird
{
	AccountConfigDialog::AccountConfigDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
	}

	QString AccountConfigDialog::GetUser () const
	{
		return Ui_.User_->text ();
	}

	void AccountConfigDialog::SetUser (const QString& user)
	{
		Ui_.User_->setText (user);
	}

	QString AccountConfigDialog::GetNick () const
	{
		return Ui_.Nick_->text ();
	}

	void AccountConfigDialog::SetNick (const QString& nick)
	{
		Ui_.Nick_->setText (nick);
	}
}
}
}
