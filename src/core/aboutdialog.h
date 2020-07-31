/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_aboutdialog.h"

namespace LC
{
	class AboutDialog : public QDialog
	{
		Q_OBJECT

		Ui::AboutDialog Ui_;
	public:
		AboutDialog (QWidget* = nullptr);
	private:
		void SetAuthors ();
		void BuildDiagInfo ();
	};
}
