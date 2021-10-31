/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "newnickservidentifydialog.h"
#include "localtypes.h"

namespace LC::Azoth::Acetamide
{
	NewNickServIdentifyDialog::NewNickServIdentifyDialog (QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);
		setAttribute (Qt::WA_DeleteOnClose);
	}

	NickServIdentify NewNickServIdentifyDialog::GetIdentify () const
	{
		return
		{
			.Server_ = Ui_.Server_->text (),
			.Nick_ = Ui_.NickName_->text (),
			.NickServNick_ = Ui_.NickServNickname_->text (),
			.AuthString_ = Ui_.NickServAuthString_->text (),
			.AuthMessage_ = Ui_.AuthMessage_->text (),
		};
	}

	void NewNickServIdentifyDialog::SetIdentify (const NickServIdentify& identify)
	{
		Ui_.Server_->setText (identify.Server_);
		Ui_.NickName_->setText (identify.Nick_);
		Ui_.NickServNickname_->setText (identify.NickServNick_);
		Ui_.NickServAuthString_->setText (identify.AuthString_);
		Ui_.AuthMessage_->setText (identify.AuthMessage_);
	}
}
