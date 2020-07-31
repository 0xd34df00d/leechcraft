/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sendmessagedialog.h"
#include <QMessageBox>
#include <QPushButton>
#include <util/util.h>
#include "ljaccount.h"
#include "ljprofile.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	SendMessageDialog::SendMessageDialog (LJProfile *profile, QWidget *parent) 
	: QDialog (parent)
	, Account_ (0)
	, Profile_ (profile)
	{
		Ui_.setupUi (this);
		Account_ = qobject_cast<LJAccount*> (Profile_->GetParentAccount ());
		Ui_.ButtonBox_->addButton (tr ("Send"), QDialogButtonBox::AcceptRole);
	}
	
	void SendMessageDialog::accept ()
	{
		if (Ui_.Addresses_->text ().isEmpty ())
		{
			QMessageBox::warning (this, "LeechCraft",
					tr ("Please enter a valid username"));
			return;
		}
		else if (Account_)
		{
			const auto& addresses = GetAddresses ();
			if (addresses.count () == 1 && addresses.at (0) == Account_->GetOurLogin ())
			{
				QMessageBox::warning (this, "LeechCraft",
					tr ("Stop trying to message yourself, livejournal is not that kind of service"));
				return;
			}
		}

		QDialog::accept ();
	}

	QStringList SendMessageDialog::GetAddresses () const
	{
		return Ui_.Addresses_->text ().split (',');
	}

	void SendMessageDialog::SetAddresses (const QStringList& addresses)
	{
		Ui_.Addresses_->setText (addresses.join (","));
	}

	QString SendMessageDialog::GetSubject () const
	{
		return Ui_.Subject_->text ();
	}

	QString SendMessageDialog::GetText () const
	{
		return Ui_.Text_->toPlainText ();
	}
}
}
}
