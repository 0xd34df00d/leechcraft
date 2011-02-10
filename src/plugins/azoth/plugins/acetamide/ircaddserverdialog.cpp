/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#include "ircaddserverdialog.h"
#include <QPushButton>
#include <QCheckBox>

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcAddServerDialog::IrcAddServerDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		
		Ui_.ControlButtons_->button (QDialogButtonBox::Ok)->setEnabled (false);
		
		connect (Ui_.Server_,
				SIGNAL (textChanged (const QString&)),
				this,
				SLOT (handleServerChanged (const QString&)));
		
		connect (Ui_.Port_,
				SIGNAL (valueChanged (int)),
				this,
				SLOT (handlePortChanged (int)));
	}

	QString IrcAddServerDialog::GetServer () const
	{
		return Ui_.Server_->text ();
	}

	void IrcAddServerDialog::SetServer (const QString& server)
	{
		Ui_.Server_->setText (server);
	}

	int IrcAddServerDialog::GetPort () const
	{
		return Ui_.Port_->value ();
	}

	void IrcAddServerDialog::SetPort (int port)
	{
		Ui_.Port_->setValue (port);
	}

	QString IrcAddServerDialog::GetPassword () const
	{
		return Ui_.Password_->text ();
	}

	void IrcAddServerDialog::SetPassword (const QString& pass)
	{
		Ui_.Password_->setText (pass);
	}
	
	void IrcAddServerDialog::SetSSL (bool ssl)
	{
		Ui_.SSL_->setChecked (ssl);
	}

	bool IrcAddServerDialog::GetSSL () const
	{
		return Ui_.SSL_->isChecked ();
	}

	void IrcAddServerDialog::handleServerChanged (const QString& text)
	{
		Ui_.ControlButtons_->button (QDialogButtonBox::Ok)->
									setEnabled (Ui_.Port_->value () && !text.isEmpty ());
	}

	void IrcAddServerDialog::handlePortChanged (int value)
	{
		Ui_.ControlButtons_->button (QDialogButtonBox::Ok)->
									setEnabled (value && !Ui_.Server_->text ().isEmpty ());
	}
};
};
};