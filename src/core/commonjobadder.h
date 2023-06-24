/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QDialog>
#include "ui_commonjobadder.h"

namespace LC
{
	class CommonJobAdder : public QDialog
	{
		Q_DECLARE_TR_FUNCTIONS (LC::CommonJobAdder)

		Ui::CommonJobAdder Ui_;
	public:
		explicit CommonJobAdder (QWidget *parent = nullptr);
	private:
		void Browse ();
		void AddJob ();
	};
}
