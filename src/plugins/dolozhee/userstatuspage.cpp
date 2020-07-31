/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "userstatuspage.h"
#include <QNetworkAccessManager>
#include <QtDebug>
#include "reportwizard.h"
#include "chooseuserpage.h"
#include "xmlgenerator.h"

namespace LC
{
namespace Dolozhee
{
	UserStatusPage::UserStatusPage (QWidget *parent)
	: QWizardPage (parent)
	{
		Ui_.setupUi (this);
	}

	void UserStatusPage::initializePage ()
	{
		auto cup = static_cast<ReportWizard*> (wizard ())->GetChooseUserPage ();

		const auto& login = cup->GetLogin ();
		const auto& pass = cup->GetPassword ();

		if (cup->GetUser () == ChooseUserPage::User::New)
			RegisterUser (login, pass, cup);
	}

	void UserStatusPage::RegisterUser (const QString& login,
			const QString& pass, ChooseUserPage *page)
	{
		Ui_.Info_->setText (tr ("Registering %1...").arg (login));

		const auto& data = XMLGenerator ().RegisterUser (login, pass,
				page->GetEmail (), page->GetFirstName (), page->GetLastName ());
		qWarning () << Q_FUNC_INFO << "unimplemented yet" << data;
	}
}
}
