/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "account.h"
#include <QUuid>
#include <QDataStream>
#include "accountconfigdialog.h"

namespace LeechCraft
{
namespace Snails
{
	Account::Account (QObject *parent)
	: QObject (parent)
	, ID_ (QUuid::createUuid ().toByteArray ())
	{
	}

	QString Account::GetName () const
	{
		return AccName_;
	}

	QString Account::GetServer () const
	{
		return InType_ == ITMaildir ?
			QString () :
			InHost_ + ':' + QString::number (InPort_);
	}

	QString Account::GetType () const
	{
		switch (InType_)
		{
		case ITIMAP:
			return "IMAP";
		case ITPOP3:
			return "POP3";
		case ITMaildir:
			return "Maildir";
		default:
			return "<unknown>";
		}
	}

	QByteArray Account::Serialize () const
	{
		QByteArray result;
		QDataStream out (&result, QIODevice::WriteOnly);
		return result;
	}

	void Account::Deserialize (const QByteArray&)
	{
	}

	void Account::OpenConfigDialog ()
	{
		std::unique_ptr<AccountConfigDialog> dia (new AccountConfigDialog);

		dia->SetName (AccName_);
		dia->SetLogin (Login_);
		dia->SetUseTLS (UseTLS_);
		dia->SetTLSRequired (TLSRequired_);
		dia->SetSMTPAuth (SMTPNeedsAuth_);
		dia->SetAPOP (APOP_);
		dia->SetAPOPRequired (APOPFail_);
		dia->SetInHost (InHost_);
		dia->SetInPort (InPort_);
		dia->SetOutHost (OutHost_);
		dia->SetOutPort (OutPort_);
		dia->SetOutLogin (OutLogin_);
		dia->SetInType (InType_);
		dia->SetOutType (OutType_);

		if (dia->exec () != QDialog::Accepted)
			return;

		AccName_ = dia->GetName ();
		Login_ = dia->GetLogin ();
		UseTLS_ = dia->GetUseTLS ();
		TLSRequired_ = dia->GetTLSRequired ();
		SMTPNeedsAuth_ = dia->GetSMTPAuth ();
		APOP_ = dia->GetAPOP ();
		APOPFail_ = dia->GetAPOPRequired ();
		InHost_ = dia->GetInHost ();
		InPort_ = dia->GetInPort ();
		OutHost_ = dia->GetOutHost ();
		OutPort_ = dia->GetOutPort ();
		OutLogin_ = dia->GetOutLogin ();
		InType_ = dia->GetInType ();
		OutType_ = dia->GetOutType ();
	}

	bool Account::IsNull () const
	{
		return AccName_.isEmpty () ||
			Login_.isEmpty ();
	}
}
}
