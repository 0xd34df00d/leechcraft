/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_profiledialog.h"

namespace LC
{
namespace Blogique
{
	class IAccount;
	class IProfileWidget;

	class ProfileDialog : public QDialog
	{
		Q_OBJECT

		Ui::ProfileDialog Ui_;

		IAccount *Account_;
		IProfileWidget *ProfileWidget_;
	public:
		ProfileDialog (IAccount *acc, QWidget *parent = 0);

	private slots:
		void handleProfileUpdated ();
	};
}
}
