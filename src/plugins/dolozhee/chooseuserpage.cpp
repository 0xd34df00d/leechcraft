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

#include "chooseuserpage.h"
#include <QSettings>
#include <QtDebug>
#include <util/passutils.h>
#include "reportwizard.h"

namespace LeechCraft
{
namespace Dolozhee
{
	ChooseUserPage::ChooseUserPage (QWidget *parent)
	: QWizardPage (parent)
	{
		Ui_.setupUi (this);

		Q_FOREACH (QRadioButton *but, findChildren<QRadioButton*> ())
			connect (but,
					SIGNAL (toggled (bool)),
					this,
					SIGNAL (completeChanged ()));
		Q_FOREACH (QLineEdit *edit, findChildren<QLineEdit*> ())
			connect (edit,
					SIGNAL (textChanged (QString)),
					this,
					SIGNAL (completeChanged ()));
	}

	void ChooseUserPage::initializePage ()
	{
		connect (wizard (),
				SIGNAL (accepted ()),
				this,
				SLOT (saveCredentials ()));

		QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Dolozhee");
		settings.beginGroup ("Credentials");
		const QString& login = settings.value ("Login").toString ();
		settings.endGroup ();

		if (login.isEmpty ())
			return;

		Ui_.Login_->setText (login);

		const QString& text = tr ("Please enter password for user %1:")
				.arg (login);
		qDebug () << Q_FUNC_INFO << GetPassKey ();
		const QString& pass = Util::GetPassword (GetPassKey (), text, this);
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

	void ChooseUserPage::saveCredentials ()
	{
		QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Dolozhee");
		settings.beginGroup ("Credentials");
		settings.setValue ("Login", GetLogin ());
		settings.endGroup ();

		Util::SavePassword (GetPassword (), GetPassKey (), this);
	}
}
}
