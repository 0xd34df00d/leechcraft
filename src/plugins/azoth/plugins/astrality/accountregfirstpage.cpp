/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountregfirstpage.h"
#include <ProtocolInfo>

namespace LC
{
namespace Azoth
{
namespace Astrality
{
	AccountRegFirstPage::AccountRegFirstPage (const Tp::ProtocolInfo& info, bool regNew, QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);

		auto visRow = [&info] (QString parm, QWidget *w1, QWidget *w2)
		{
			if (!info.hasParameter (parm))
			{
				w1->hide ();
				if (w2)
					w2->hide ();
			}
		};
		visRow ("account", Ui_.AccIDLabel_, Ui_.AccID_);
		visRow ("server", Ui_.ServerLabel_, Ui_.Server_);
		visRow ("port", Ui_.PortLabel_, Ui_.Port_);
		visRow ("require-encryption", Ui_.RequireEncryption_, 0);

		if (!regNew)
		{
			Ui_.PasswordLabel_->hide ();
			Ui_.Password_->hide ();
		}
	}

	void AccountRegFirstPage::SetParams (const QVariantMap& params)
	{
		auto setStr = [&params] (QLineEdit *edit, const QString& name)
		{
			edit->setText (params [name].toString ());
		};
		setStr (Ui_.AccID_, "account");
		setStr (Ui_.Server_, "server");

		Ui_.Port_->setValue (params ["port"].toInt ());

		Ui_.RequireEncryption_->setCheckState (params ["require-encryption"].toBool () ? Qt::Checked : Qt::Unchecked);
	}

	QString AccountRegFirstPage::GetAccountID () const
	{
		return Ui_.AccID_->text ();
	}

	QString AccountRegFirstPage::GetPassword () const
	{
		return Ui_.Password_->text ();
	}

	QString AccountRegFirstPage::GetServer () const
	{
		return Ui_.Server_->text ();
	}

	int AccountRegFirstPage::GetPort () const
	{
		return Ui_.Port_->value ();
	}

	bool AccountRegFirstPage::ShouldRequireEncryption () const
	{
		return Ui_.RequireEncryption_->isChecked ();
	}

	void AccountRegFirstPage::SetSettings (const AccountWrapper::Settings& s)
	{
		Ui_.Autodisconnect_->setCheckState (s.Autodisconnect_ ? Qt::Checked : Qt::Unchecked);
	}

	void AccountRegFirstPage::Augment (AccountWrapper::Settings& s) const
	{
		s.Autodisconnect_ = Ui_.Autodisconnect_->checkState () == Qt::Checked;
	}
}
}
}
