/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizardPage>
#include <QUrl>
#include "ui_startupfirstpage.h"

namespace LC
{
namespace Poshuku
{
namespace CleanWeb
{
	class Core;

	class StartupFirstPage : public QWizardPage
	{
		Q_OBJECT

		Ui::StartupFirstPageWidget Ui_;

		Core * const Core_;
	public:
		StartupFirstPage (Core *core, QWidget* = 0);

		void initializePage ();
	private slots:
		void handleAccepted ();
	};
}
}
}
