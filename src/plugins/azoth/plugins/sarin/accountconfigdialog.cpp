/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountconfigdialog.h"
#include "toxaccountconfiguration.h"

namespace LC::Azoth::Sarin
{
	AccountConfigDialog::AccountConfigDialog (QDialog *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);
	}

	ToxAccountConfiguration AccountConfigDialog::GetConfig () const
	{
		return
		{
			Ui_.AllowUDP_->checkState () == Qt::Checked,
			Ui_.AllowIPv6_->checkState () == Qt::Checked,
			Ui_.ProxyHost_->text ().trimmed (),
			Ui_.ProxyPort_->value ()
		};
	}

	void AccountConfigDialog::SetConfig (const ToxAccountConfiguration& config)
	{
		Ui_.AllowUDP_->setCheckState (config.AllowUDP_ ? Qt::Checked : Qt::Unchecked);
		Ui_.AllowIPv6_->setCheckState (config.AllowIPv6_ ? Qt::Checked : Qt::Unchecked);
		Ui_.ProxyHost_->setText (config.ProxyHost_);
		Ui_.ProxyPort_->setValue (config.ProxyPort_);
	}
}
