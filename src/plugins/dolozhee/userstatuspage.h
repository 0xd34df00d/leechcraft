/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizardPage>
#include "ui_userstatuspage.h"

namespace LC
{
namespace Dolozhee
{
	class ChooseUserPage;

	class UserStatusPage : public QWizardPage
	{
		Ui::UserStatusPage Ui_;
	public:
		explicit UserStatusPage (QWidget* = nullptr);

		void initializePage () override;
	private:
		void RegisterUser (const QString&, const QString&, ChooseUserPage*);
	};
}
}
