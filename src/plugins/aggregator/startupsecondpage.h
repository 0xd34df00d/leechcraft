/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AGGREGATOR_STARTUPSECONDPAGE_H
#define PLUGINS_AGGREGATOR_STARTUPSECONDPAGE_H
#include <QWizardPage>
#include "ui_startupsecondpage.h"

namespace LC
{
namespace Util
{
	class BackendSelector;
};

namespace Aggregator
{
	class StartupSecondPage : public QWizardPage
	{
		Q_OBJECT

		Ui::StartupSecondPageWidget Ui_;
		Util::BackendSelector *Selector_;
	public:
		StartupSecondPage (QWidget* = 0);

		void initializePage ();
	};
}
}

#endif
