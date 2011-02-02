/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "addcontactdialog.h"
#include "interfaces/iprotocol.h"
#include "interfaces/iaccount.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
	AddContactDialog::AddContactDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		Q_FOREACH (IProtocol *proto, Core::Instance ().GetProtocols ())
			Ui_.Protocol_->addItem (proto->GetProtocolName (),
					QVariant::fromValue<IProtocol*> (proto));
	}

	IAccount* AddContactDialog::GetSelectedAccount () const
	{
		int idx = Ui_.Account_->currentIndex ();
		return idx >= 0 ?
			Ui_.Account_->itemData (idx).value<IAccount*> () :
			0;
	}

	QString AddContactDialog::GetContactID () const
	{
		return Ui_.ContactID_->text ();
	}

	QString AddContactDialog::GetNick () const
	{
		return Ui_.Nick_->text ();
	}

	QString AddContactDialog::GetReason () const
	{
		return Ui_.Reason_->toPlainText ();
	}

	QStringList AddContactDialog::GetGroups () const
	{
		QStringList result;
		Q_FOREACH (const QString& str, Ui_.Groups_->text ().split (';'))
			result << str.trimmed ();
		return result;
	}

	void AddContactDialog::on_Protocol__currentIndexChanged (int idx)
	{
		Ui_.Account_->clear ();
		if (idx < 0)
			return;

		IProtocol *proto = Ui_.Protocol_->
				itemData (idx).value<IProtocol*> ();
		Q_FOREACH (QObject *accObj, proto->GetRegisteredAccounts ())
		{
			IAccount *acc = qobject_cast<IAccount*> (accObj);
			if (!acc)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast"
						<< accObj
						<< "to IAccount";
				continue;
			}

			if (acc->GetState ().State_ == SOffline &&
					!(acc->GetAccountFeatures () & IAccount::FCanAddContactsInOffline))
				continue;

			Ui_.Account_->addItem (QString ("%1 (%2)")
						.arg (acc->GetAccountName ())
						.arg (acc->GetOurNick ()),
					QVariant::fromValue<IAccount*> (acc));
		}
	}
}
}
