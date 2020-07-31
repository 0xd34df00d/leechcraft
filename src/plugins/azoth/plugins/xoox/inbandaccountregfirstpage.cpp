/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "inbandaccountregfirstpage.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	InBandAccountRegFirstPage::InBandAccountRegFirstPage (QWidget *parent)
	: QWizardPage (parent)
	{
		Ui_.setupUi (this);
		connect (Ui_.ServerName_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SIGNAL (completeChanged ()));
		connect (Ui_.ServerName_,
				SIGNAL (editTextChanged (const QString&)),
				this,
				SIGNAL (completeChanged ()));
	}
	
	QString InBandAccountRegFirstPage::GetServerName () const
	{
		return Ui_.ServerName_->currentText ();
	}
	
	bool InBandAccountRegFirstPage::isComplete () const
	{
		if (Ui_.ServerName_->currentText ().isEmpty ())
			return false;
		
		return QWizardPage::isComplete ();
	}
}
}
}
