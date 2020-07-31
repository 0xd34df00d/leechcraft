/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizardPage>
#include "ui_addaccountwizardfirstpage.h"

namespace LC
{
namespace Azoth
{
	class AddAccountWizardFirstPage : public QWizardPage
	{
		Q_OBJECT
		
		Ui::AddAccountWizardFirstPage Ui_;
		QList<QWidget*> Widgets_;
	public:
		AddAccountWizardFirstPage (QWidget* = nullptr);
		
		void initializePage () override;
	private:
		void CleanupWidgets ();
	private slots:
		void readdWidgets ();
		void handleAccepted ();
	};
}
}
