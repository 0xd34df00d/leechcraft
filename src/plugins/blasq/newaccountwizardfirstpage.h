/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizardPage>
#include "ui_newaccountwizardfirstpage.h"

namespace LC
{
namespace Blasq
{
	class ServicesManager;
	class IService;

	class NewAccountWizardFirstPage : public QWizardPage
	{
		Q_OBJECT

		Ui::NewAccountWizardFirstPage Ui_;

		const ServicesManager * const ServicesMgr_;

		IService *Service_ = 0;
		QList<QWidget*> Widgets_;
	public:
		NewAccountWizardFirstPage (const ServicesManager*, QWidget* = 0);

		void initializePage ();
	private slots:
		void updatePages ();
		void handleAccepted ();
	};
}
}
