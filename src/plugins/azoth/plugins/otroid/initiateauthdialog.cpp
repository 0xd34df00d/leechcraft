/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "initiateauthdialog.h"
#include <QtDebug>
#include <interfaces/azoth/iclentry.h>
#include <util/azoth/util.h>
#include "authenticator.h"

namespace LC
{
namespace Azoth
{
namespace OTRoid
{
	InitiateAuthDialog::InitiateAuthDialog (ICLEntry *entry)
	{
		Ui_.setupUi (this);
		Ui_.TextLabel_->setText (tr ("Choose authentication method for %1:")
					.arg (FormatNameWithAddress (*entry)));
	}

	SmpMethod InitiateAuthDialog::GetMethod () const
	{
		switch (Ui_.MethodBox_->currentIndex ())
		{
		case 0:
			return SmpMethod::Question;
		case 1:
			return SmpMethod::SharedSecret;
		default:
			qWarning () << Q_FUNC_INFO
					<< "unknown UI method";
			return SmpMethod::SharedSecret;
		}
	}

	QString InitiateAuthDialog::GetQuestion () const
	{
		return Ui_.Question_->text ();
	}

	QString InitiateAuthDialog::GetAnswer () const
	{
		return Ui_.Answer_->text ();
	}

	void InitiateAuthDialog::on_MethodBox__currentIndexChanged ()
	{
		Ui_.Question_->setEnabled (GetMethod () == SmpMethod::Question);
	}
}
}
}
