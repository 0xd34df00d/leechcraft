/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizardPage>
#include "ui_registerpage.h"

namespace LC
{
namespace Blasq
{
namespace Vangog
{
	class RegisterPage : public QWizardPage
	{
		Q_OBJECT

		Ui::RegisterPage Ui_;

	public:
		RegisterPage (QWidget *parent = 0);

		QString GetLogin () const;
	};
}
}
}
