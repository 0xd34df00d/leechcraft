/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AGGREGATOR_STARTUPFIRSTPAGE_H
#define PLUGINS_AGGREGATOR_STARTUPFIRSTPAGE_H
#include <QWizardPage>
#include "ui_startupfirstpage.h"

namespace LC
{
namespace Aggregator
{
	class StartupFirstPage : public QWizardPage
	{
		Q_OBJECT

		Ui::StartupFirstPageWidget Ui_;
	public:
		StartupFirstPage (QWidget* = 0);

		void initializePage ();
	private slots:
		void handleAccepted ();
	};
}
}

#endif
