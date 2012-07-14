/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "newnickservidentifydialog.h"
#include <QMessageBox>

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	NewNickServIdentifyDialog::NewNickServIdentifyDialog (QWidget *parent, Qt::WindowFlags f)
	: QDialog (parent, f)
	{
		Ui_.setupUi (this);
	}

	QString NewNickServIdentifyDialog::GetServer () const
	{
		return Ui_.Server_->text ();
	}

	QString NewNickServIdentifyDialog::GetNickName () const
	{
		return Ui_.NickName_->text ();
	}

	QString NewNickServIdentifyDialog::GetNickServNickName () const
	{
		return Ui_.NickServNickname_->text ();
	}

	QString NewNickServIdentifyDialog::GetAuthString () const
	{
		return Ui_.NickServAuthString_->text ();
	}

	QString NewNickServIdentifyDialog::GetAuthMessage () const
	{
		return Ui_.AuthMessage_->text ();
	}

	void NewNickServIdentifyDialog::accept ()
	{
		if (GetServer ().isEmpty () ||
				GetNickName ().isEmpty () ||
				GetNickServNickName ().isEmpty () ||
				GetAuthString ().isEmpty () ||
				GetAuthMessage ().isEmpty ())
			return;

		QDialog::accept ();
	}

	void NewNickServIdentifyDialog::SetServer (const QString& server)
	{
		Ui_.Server_->setText (server);
	}

	void NewNickServIdentifyDialog::SetNickName (const QString& nick)
	{
		Ui_.NickName_->setText (nick);
	}

	void NewNickServIdentifyDialog::SetNickServNickName (const QString& nickServ)
	{
		Ui_.NickServNickname_->setText (nickServ);
	}

	void NewNickServIdentifyDialog::SetAuthString (const QString& authString)
	{
		Ui_.NickServAuthString_->setText (authString);
	}

	void NewNickServIdentifyDialog::SetAuthMessage (const QString& authMessage)
	{
		Ui_.AuthMessage_->setText (authMessage);
	}

}
}
}