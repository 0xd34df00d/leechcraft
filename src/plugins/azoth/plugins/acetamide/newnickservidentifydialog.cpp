/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "newnickservidentifydialog.h"
#include <QMessageBox>

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	NewNickServIdentifyDialog::NewNickServIdentifyDialog (QWidget *parent)
	: QDialog (parent)
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
