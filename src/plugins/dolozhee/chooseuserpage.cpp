/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "chooseuserpage.h"
#include <QSettings>
#include <QtDebug>
#include <util/xpc/passutils.h>
#include "reportwizard.h"

namespace LC
{
namespace Dolozhee
{
	ChooseUserPage::ChooseUserPage (const ICoreProxy_ptr& proxy, QWidget *parent)
	: QWizardPage (parent)
	, Proxy_ (proxy)
	{
		Ui_.setupUi (this);

		for (const auto but : findChildren<QRadioButton*> ())
			connect (but,
					&QRadioButton::toggled,
					this,
					&ChooseUserPage::completeChanged);
		for (const auto edit : findChildren<QLineEdit*> ())
			connect (edit,
					&QLineEdit::textChanged,
					this,
					&ChooseUserPage::completeChanged);
	}

	void ChooseUserPage::initializePage ()
	{
		connect (wizard (),
				&QDialog::accepted,
				this,
				&ChooseUserPage::SaveCredentials);

		QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Dolozhee");
		settings.beginGroup ("Credentials");
		const QString& login = settings.value ("Login").toString ();
		settings.endGroup ();

		if (login.isEmpty ())
			return;

		Ui_.Existing_->setChecked (true);

		Ui_.Login_->setText (login);

		const QString& text = tr ("Please enter password for user %1:")
				.arg (login);
		const QString& pass = Util::GetPassword (GetPassKey (), text, Proxy_);
		Ui_.Password_->setText (pass);
	}

	int ChooseUserPage::nextId () const
	{
		return GetUser () != User::New ?
				ReportWizard::PageID::ReportType :
				ReportWizard::PageID::UserStatus;
	}

	bool ChooseUserPage::isComplete () const
	{
		switch (GetUser ())
		{
		case User::Anonymous:
			return true;
		case User::Existing:
			return !GetLogin ().isEmpty () && !GetPassword ().isEmpty ();
		default:
			return false;
		}
	}

	ChooseUserPage::User ChooseUserPage::GetUser () const
	{
		if (Ui_.New_->isChecked ())
			return User::New;
		else if (Ui_.Existing_->isChecked ())
			return User::Existing;
		else
			return User::Anonymous;
	}

	QString ChooseUserPage::GetLogin () const
	{
		return GetUser () == User::Anonymous ?
				"7b034124da8534c8e3464afd4dd59abc244bd271" :
				Ui_.Login_->text ();
	}

	QString ChooseUserPage::GetPassword () const
	{
		return GetUser () == User::Anonymous ?
				"somepass" :
				Ui_.Password_->text ();
	}

	QString ChooseUserPage::GetEmail () const
	{
		return Ui_.EMail_->text ();
	}

	QString ChooseUserPage::GetFirstName () const
	{
		return Ui_.FirstName_->text ();
	}

	QString ChooseUserPage::GetLastName () const
	{
		return Ui_.LastName_->text ();
	}

	QString ChooseUserPage::GetPassKey () const
	{
		return "org.LeechCraft.Dolozhee.Username_" + Ui_.Login_->text ();
	}

	void ChooseUserPage::SaveCredentials ()
	{
		if (GetUser () != User::Existing)
			return;

		QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Dolozhee");
		settings.beginGroup ("Credentials");
		settings.setValue ("Login", GetLogin ());
		settings.endGroup ();

		Util::SavePassword (GetPassword (), GetPassKey (), Proxy_);
	}
}
}
