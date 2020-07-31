/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "registerpage.h"

namespace LC
{
namespace Blasq
{
namespace DeathNote
{
	RegisterPage::RegisterPage (QWidget *parent)
	: QWizardPage (parent)
	{
		Ui_.setupUi (this);
	}

	QString RegisterPage::GetLogin () const
	{
		return Ui_.Login_->text ();
	}

	QString RegisterPage::GetPassword () const
	{
		return Ui_.Password_->text ();
	}

}
}
}
