/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QWizardPage>
#include "ui_startupfirstpage.h"

namespace LC::BitTorrent
{
	class SessionSettingsManager;

	class StartupFirstPage : public QWizardPage
	{
		Q_DECLARE_TR_FUNCTIONS (LC::BitTorrent::StartupFirstPage)

		Ui::StartupFirstPageWidget Ui_;

		SessionSettingsManager * const SSM_;
	public:
		explicit StartupFirstPage (SessionSettingsManager*, QWidget* = nullptr);

		void initializePage () override;
	};
}
