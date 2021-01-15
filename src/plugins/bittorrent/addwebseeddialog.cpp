/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "addwebseeddialog.h"

namespace LC
{
namespace BitTorrent
{
	AddWebSeedDialog::AddWebSeedDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
	}

	QString AddWebSeedDialog::GetURL () const
	{
		return Ui_.URL_->text ();
	}

	WebSeedType AddWebSeedDialog::GetType () const
	{
		return Ui_.BEP19_->isChecked () ? WebSeedType::Bep19 : WebSeedType::Bep17;
	}
}
}
