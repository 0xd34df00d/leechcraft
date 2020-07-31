/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/gui/pagenotification.h>
#include "pageformsdata.h"
#include "ui_passwordremember.h"

namespace LC
{
namespace Poshuku
{
	class PasswordRemember : public Util::PageNotification
	{
		Q_OBJECT

		Ui::PasswordRemember Ui_;
		PageFormsData_t TempData_;
	public:
		PasswordRemember (QWidget* = 0);
	public slots:
		void add (const PageFormsData_t&);
	private slots:
		void on_Remember__released ();
		void on_NotNow__released ();
		void on_Never__released ();
	};
}
}
