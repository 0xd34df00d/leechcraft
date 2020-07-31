/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_userfilters.h"

namespace LC
{
namespace Poshuku
{
namespace CleanWeb
{
	class UserFiltersModel;

	class UserFilters : public QWidget
	{
		Q_OBJECT

		Ui::UserFilters Ui_;
		UserFiltersModel * const Model_;
	public:
		UserFilters (UserFiltersModel*, QWidget* = 0);
	private slots:
		void on_Add__released ();
		void on_Modify__released ();
		void on_Remove__released ();
		void on_Paste__released ();
		void on_Load__released ();

		void accept ();
		void reject ();
	};
}
}
}
