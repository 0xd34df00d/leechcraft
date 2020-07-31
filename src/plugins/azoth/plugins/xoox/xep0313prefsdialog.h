/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_xep0313prefsdialog.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class Xep0313Manager;
	class Xep0313PrefIq;

	class Xep0313PrefsDialog : public QDialog
	{
		Q_OBJECT

		Ui::Xep0313PrefsDialog Ui_;
		Xep0313Manager * const Manager_;
	public:
		Xep0313PrefsDialog (Xep0313Manager*, QWidget* = nullptr);
	private slots:
		void updatePrefs ();
		void handlePrefs (const Xep0313PrefIq&);
	};
}
}
}
