/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "replacedialog.h"

namespace LC
{
namespace Popishu
{
	ReplaceDialog::ReplaceDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
	}

	QString ReplaceDialog::GetBefore () const
	{
		return Ui_.Before_->text ();
	}

	QString ReplaceDialog::GetAfter () const
	{
		return Ui_.After_->text ();
	}

	Qt::CaseSensitivity ReplaceDialog::GetCaseSensitivity () const
	{
		return Ui_.CaseSensitive_->isChecked () ?
				Qt::CaseSensitive :
				Qt::CaseInsensitive;
	}

	ReplaceDialog::Scope ReplaceDialog::GetScope () const
	{
		if (Ui_.ScopeSelected_->isChecked ())
			return Scope::Selected;
		else
			return Scope::All;
	}
}
}
