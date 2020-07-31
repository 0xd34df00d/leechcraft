/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "banpeersdialog.h"
#include "ipvalidators.h"

namespace LC
{
namespace BitTorrent
{
	BanPeersDialog::BanPeersDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		Ui_.IP4Start_->setValidator (new ValidateIPv4 (this));
		Ui_.IP4End_->setValidator (new ValidateIPv4 (this));
		Ui_.IP6Start_->setValidator (new ValidateIPv6 (this));
		Ui_.IP6End_->setValidator (new ValidateIPv6 (this));
	}

	void BanPeersDialog::SetIP (const QString& ip)
	{
		if (ip.contains (":"))
		{
			Ui_.RadioIP6_->setChecked (true);
			Ui_.IP6Start_->setText (ip);
			Ui_.IP6End_->setText (ip);
		}
		else
		{
			Ui_.IP4Start_->setText (ip);
			Ui_.IP4End_->setText (ip);
		}
	}

	void BanPeersDialog::SetIP (const QString& first, const QString& last)
	{
		if (first.contains (":"))
		{
			Ui_.RadioIP6_->setChecked (true);
			Ui_.IP6Start_->setText (first);
			Ui_.IP6End_->setText (last);
		}
		else
		{
			Ui_.IP4Start_->setText (first);
			Ui_.IP4End_->setText (last);
		}
	}

	QString BanPeersDialog::GetStart () const
	{
		if (Ui_.IP4Start_->isEnabled ())
			return Ui_.IP4Start_->hasAcceptableInput () ?
				Ui_.IP4Start_->text () :
				QString ();
		else
			return Ui_.IP6Start_->hasAcceptableInput () ?
				Ui_.IP6Start_->text () :
				QString ();
	}

	QString BanPeersDialog::GetEnd () const
	{
		if (Ui_.IP4Start_->isEnabled ())
			return Ui_.IP4End_->hasAcceptableInput () ?
				Ui_.IP4End_->text () :
				QString ();
		else
			return Ui_.IP6End_->hasAcceptableInput () ?
				Ui_.IP6End_->text () :
				QString ();
	}
}
}
