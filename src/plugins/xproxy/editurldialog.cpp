/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "editurldialog.h"
#include "structures.h"

namespace LC
{
namespace XProxy
{
	EditUrlDialog::EditUrlDialog (QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);
	}

	ReqTarget EditUrlDialog::GetReqTarget () const
	{
		auto rxPat = Ui_.TargetHost_->text ();
		if (!rxPat.contains ("*") && !rxPat.contains ("^") && !rxPat.contains ("$"))
		{
			rxPat.prepend (".*");
			rxPat.append (".*");
		}

		const Util::RegExp rx { rxPat, Qt::CaseInsensitive };
		return
		{
			rx,
			Ui_.TargetPort_->value (),
			Ui_.TargetProto_->text ().split (" ", Qt::SkipEmptyParts)
		};
	}

	void EditUrlDialog::SetReqTarget (const ReqTarget& target)
	{
		Ui_.TargetHost_->setText (target.Host_.GetPattern ());
		Ui_.TargetPort_->setValue (target.Port_);
		Ui_.TargetProto_->setText (target.Protocols_.join (" "));
	}
}
}
