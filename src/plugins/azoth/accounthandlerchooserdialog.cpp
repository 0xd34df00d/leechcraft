/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "accounthandlerchooserdialog.h"
#include <QtDebug>
#include "interfaces/iaccount.h"

namespace LeechCraft
{
namespace Azoth
{
	AccountHandlerChooserDialog::AccountHandlerChooserDialog (const QList<QObject*>& accounts,
			const QString& text, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		Ui_.Text_->setText (text);
		
		Q_FOREACH (QObject *accObj, accounts)
		{
			IAccount *acc = qobject_cast<IAccount*> (accObj);
			if (!acc)
			{
				qWarning () << Q_FUNC_INFO
						<< "account doesn't implement IAccount"
						<< accObj;
				continue;
			}
			
			Ui_.AccountsBox_->addItem (acc->GetAccountName (),
					QVariant::fromValue<QObject*> (accObj));
		}
	}
	
	QObject* AccountHandlerChooserDialog::GetSelectedAccount () const
	{
		const int idx = Ui_.AccountsBox_->currentIndex ();
		if (idx < 0)
			return 0;
		
		return Ui_.AccountsBox_->itemData (idx).value<QObject*> ();
	}
}
}
