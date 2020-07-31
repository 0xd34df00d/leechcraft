/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountconfigdialog.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	AccountConfigDialog::AccountConfigDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		connect (Ui_.Reauth_,
				SIGNAL (released ()),
				this,
				SIGNAL (reauthRequested ()));
	}

	bool AccountConfigDialog::GetFileLogEnabled () const
	{
		return Ui_.KeepFileLog_->checkState () == Qt::Checked;
	}

	void AccountConfigDialog::SetFileLogEnabled (bool enabled)
	{
		Ui_.KeepFileLog_->setCheckState (enabled ? Qt::Checked : Qt::Unchecked);
	}

	bool AccountConfigDialog::GetUpdateStatusEnabled () const
	{
		return Ui_.UpdateStatus_->checkState () == Qt::Checked;
	}

	void AccountConfigDialog::SetUpdateStatusEnabled (bool enabled)
	{
		Ui_.UpdateStatus_->setCheckState (enabled ? Qt::Checked : Qt::Unchecked);
	}

	bool AccountConfigDialog::GetPublishTuneEnabled () const
	{
		return Ui_.PublishTune_->checkState () == Qt::Checked;
	}

	void AccountConfigDialog::SetPublishTuneEnabled (bool enabled)
	{
		Ui_.PublishTune_->setCheckState (enabled ? Qt::Checked : Qt::Unchecked);
	}

	bool AccountConfigDialog::GetMarkAsOnline () const
	{
		return Ui_.MarkAsOnline_->checkState () == Qt::Checked;
	}

	void AccountConfigDialog::SetMarkAsOnline (bool enabled)
	{
		Ui_.MarkAsOnline_->setCheckState (enabled ? Qt::Checked : Qt::Unchecked);
	}
}
}
}
