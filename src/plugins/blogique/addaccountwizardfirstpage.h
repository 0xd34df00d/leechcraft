/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizardPage>
#include "ui_addaccountwizardfirstpage.h"

namespace LC
{
namespace Blogique
{
	class AddAccountWizardFirstPage : public QWizardPage
	{
		Q_OBJECT

		Ui::AddAccountWizardFirstPage Ui_;
		QList<QWidget*> Widgets_;

	public:
		AddAccountWizardFirstPage (QWidget* = 0);
		void initializePage ();
		bool isComplete () const;

	private slots:
		void readdWidgets ();
		void handleAccepted ();
		void handleAccountNameChanged (const QString& text);
	};
}
}
