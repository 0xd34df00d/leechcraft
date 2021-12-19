/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vcarddialog.h"
#include "entrybase.h"
#include "localtypes.h"

namespace LC::Azoth::Acetamide
{
	VCardDialog::VCardDialog (QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);
	}

	void VCardDialog::UpdateInfo (const WhoIsMessage& msg)
	{
		setWindowTitle (tr ("VCard for %1").arg (msg.Nick_));

		Ui_.EditNick_->setText (msg.Nick_);
		Ui_.EditUserName_->setText (msg.UserName_);
		Ui_.EditHostName_->setText (msg.Host_);
		Ui_.EditRealName_->setText (msg.RealName_);
		Ui_.ServerName_->setText (msg.ServerName_);
		Ui_.ServerDislocation_->setText (msg.ServerCountry_);
		Ui_.EMailAddress_->setText (msg.Mail_);
	}
}
