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

#include "accountconfigdialog.h"

namespace LeechCraft
{
namespace Snails
{
	AccountConfigDialog::AccountConfigDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		connect (Ui_.InType_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (resetInPort ()));
		connect (Ui_.UseTLS_,
				SIGNAL (stateChanged (int)),
				this,
				SLOT (resetInPort ()));
	}

	QString AccountConfigDialog::GetName () const
	{
		return Ui_.AccName_->text ();
	}

	void AccountConfigDialog::SetName (const QString& name)
	{
		Ui_.AccName_->setText (name);
	}

	QString AccountConfigDialog::GetUserName () const
	{
		return Ui_.UserName_->text ();
	}

	void AccountConfigDialog::SetUserName (const QString& name)
	{
		Ui_.UserName_->setText (name);
	}

	QString AccountConfigDialog::GetUserEmail () const
	{
		return Ui_.UserEmail_->text ();
	}

	void AccountConfigDialog::SetUserEmail (const QString& email)
	{
		Ui_.UserEmail_->setText (email);
	}

	QString AccountConfigDialog::GetLogin () const
	{
		return Ui_.InLogin_->text ();
	}

	void AccountConfigDialog::SetLogin (const QString& login)
	{
		Ui_.InLogin_->setText (login);
	}

	Account::InType AccountConfigDialog::GetInType () const
	{
		return static_cast<Account::InType> (Ui_.InType_->currentIndex ());
	}

	void AccountConfigDialog::SetInType (Account::InType type)
	{
		Ui_.InType_->setCurrentIndex (static_cast<int> (type));
	}

	QString AccountConfigDialog::GetInHost () const
	{
		return Ui_.InHost_->text ();
	}

	void AccountConfigDialog::SetInHost (const QString& host)
	{
		Ui_.InHost_->setText (host);
	}

	int AccountConfigDialog::GetInPort () const
	{
		return Ui_.InPort_->value ();
	}

	void AccountConfigDialog::SetInPort (int port)
	{
		Ui_.InPort_->setValue (port);
	}

	Account::OutType AccountConfigDialog::GetOutType () const
	{
		return static_cast<Account::OutType> (Ui_.OutType_->currentIndex ());
	}

	void AccountConfigDialog::SetOutType (Account::OutType type)
	{
		Ui_.OutType_->setCurrentIndex (static_cast<int> (type));
	}

	QString AccountConfigDialog::GetOutHost () const
	{
		return Ui_.OutAddress_->text ();
	}

	void AccountConfigDialog::SetOutHost (const QString& host)
	{
		Ui_.OutAddress_->setText (host);
	}

	int AccountConfigDialog::GetOutPort () const
	{
		return Ui_.OutPort_->value ();
	}

	void AccountConfigDialog::SetOutPort (int port)
	{
		Ui_.OutPort_->setValue (port);
	}

	QString AccountConfigDialog::GetOutLogin () const
	{
		return Ui_.CustomOut_->isChecked () ?
				Ui_.OutLogin_->text () :
				QString ();
	}

	void AccountConfigDialog::SetOutLogin (const QString& login)
	{
		Ui_.CustomOut_->setChecked (!login.isEmpty ());
		Ui_.OutLogin_->setText (login);
	}

	bool AccountConfigDialog::GetUseSASL () const
	{
		return Ui_.UseSASL_->checkState () == Qt::Checked;
	}

	void AccountConfigDialog::SetUseSASL (bool use)
	{
		Ui_.UseSASL_->setCheckState (use ? Qt::Checked : Qt::Unchecked);
	}

	bool AccountConfigDialog::GetSASLRequired () const
	{
		return Ui_.SASLRequired_->checkState () == Qt::Checked;
	}

	void AccountConfigDialog::SetSASLRequired (bool req)
	{
		Ui_.SASLRequired_->setCheckState (req ? Qt::Checked : Qt::Unchecked);
	}

	bool AccountConfigDialog::GetUseTLS () const
	{
		return Ui_.UseTLS_->checkState () == Qt::Checked;
	}

	void AccountConfigDialog::SetUseTLS (bool use)
	{
		Ui_.UseTLS_->setCheckState (use ? Qt::Checked : Qt::Unchecked);
	}

	bool AccountConfigDialog::GetTLSRequired () const
	{
		return Ui_.TLSRequired_->checkState () == Qt::Checked;
	}

	void AccountConfigDialog::SetTLSRequired (bool req)
	{
		Ui_.TLSRequired_->setCheckState (req ? Qt::Checked : Qt::Unchecked);
	}

	bool AccountConfigDialog::GetSMTPAuth () const
	{
		return Ui_.SMTPAuthRequired_->checkState () == Qt::Checked;
	}

	void AccountConfigDialog::SetSMTPAuth (bool smtp)
	{
		Ui_.SMTPAuthRequired_->setCheckState (smtp ? Qt::Checked : Qt::Unchecked);
	}

	bool AccountConfigDialog::GetAPOP () const
	{
		return Ui_.APOP_->checkState () == Qt::Checked;
	}

	void AccountConfigDialog::SetAPOP (bool apop)
	{
		Ui_.APOP_->setCheckState (apop ? Qt::Checked : Qt::Unchecked);
	}

	bool AccountConfigDialog::GetAPOPRequired () const
	{
		return Ui_.APOPRequired_->checkState () == Qt::Checked;
	}

	void AccountConfigDialog::SetAPOPRequired (bool req)
	{
		Ui_.APOPRequired_->setCheckState (req ? Qt::Checked : Qt::Unchecked);
	}

	void AccountConfigDialog::resetInPort ()
	{
		QMap<Account::InType, QMap<bool, int>> values;
		values [Account::InType::IMAP] [true] = 993;
		values [Account::InType::IMAP] [false] = 143;
		values [Account::InType::POP3] [true] = 995;
		values [Account::InType::POP3] [false] = 110;

		const Account::InType selected = static_cast<Account::InType> (Ui_.InType_->currentIndex ());
		const bool tls = Ui_.UseTLS_->checkState () == Qt::Checked;
		Ui_.InPort_->setValue (values [selected] [tls]);
	}
}
}
