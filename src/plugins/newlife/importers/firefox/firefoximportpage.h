/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizardPage>
#include "ui_feedssettingsimportpage.h"

namespace LC
{
struct Entity;

namespace NewLife
{
namespace Importers
{
	class FirefoxImportPage : public QWizardPage
	{
		Q_OBJECT

		Ui::FeedsSettingsImportPage Ui_;
	public:
		FirefoxImportPage (QWidget* = 0);

		bool CheckValidity (const QString&) const;
		bool isComplete () const override;
		void initializePage () override;
	private slots:
		void on_Browse__released ();
		void on_FileLocation__textEdited (const QString&);
		void handleAccepted (int);
	};
}
}
}
