/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "userstatuspage.h"
#include "reportwizard.h"
#include "chooseuserpage.h"
#include "xmlgenerator.h"
#include <QNetworkAccessManager>

namespace LeechCraft
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
		auto cup = qobject_cast<ReportWizard*> (wizard ())->GetChooseUserPage ();

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
		auto wiz = qobject_cast<ReportWizard*> (wizard ());
		auto reply = wiz->PostRequest ("/users.xml", data);
		Q_UNUSED (reply);
	}
}
}
