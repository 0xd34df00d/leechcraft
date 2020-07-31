/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accounthandlerchooserdialog.h"
#include <QtDebug>
#include "interfaces/azoth/iaccount.h"

namespace LC
{
namespace Azoth
{
	AccountHandlerChooserDialog::AccountHandlerChooserDialog (const QList<IAccount*>& accounts,
			const QString& text, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		Ui_.Text_->setText (text);

		for (const auto acc : accounts)
			Ui_.AccountsBox_->addItem (acc->GetAccountName (),
					QVariant::fromValue<IAccount*> (acc));
	}

	IAccount* AccountHandlerChooserDialog::GetSelectedAccount () const
	{
		const int idx = Ui_.AccountsBox_->currentIndex ();
		if (idx < 0)
			return 0;

		return Ui_.AccountsBox_->itemData (idx).value<IAccount*> ();
	}
}
}
