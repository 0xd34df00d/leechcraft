/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "addpeerdialog.h"
#include "ipvalidators.h"

namespace LC
{
namespace BitTorrent
{
	AddPeerDialog::AddPeerDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		Ui_.IP4_->setValidator (new ValidateIPv4 (this));
		Ui_.IP6_->setValidator (new ValidateIPv6 (this));
	}

	QString AddPeerDialog::GetIP () const
	{
		return Ui_.IP4_->isEnabled () ? Ui_.IP4_->text () : Ui_.IP6_->text ();
	}

	int AddPeerDialog::GetPort () const
	{
		return Ui_.Port_->value ();
	}
}
}
