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

#include "bookmarksmanagerdialog.h"
#include <QStandardItemModel>
#include "interfaces/imucjoinwidget.h"
#include "interfaces/iaccount.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
	BookmarksManagerDialog::BookmarksManagerDialog (QWidget *parent)
	: QDialog (parent)
	, BMModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.BookmarksTree_->setModel (BMModel_);
		
		Q_FOREACH (IProtocol *proto, Core::Instance ().GetProtocols ())
		{
			QWidget *widget = proto->GetMUCJoinWidget ();
			IMUCJoinWidget *joiner = qobject_cast<IMUCJoinWidget*> (widget);
			if (!joiner)
				continue;
			
			Proto2Joiner_ [proto->GetProtocolID ()] = joiner;
			
			Q_FOREACH (QObject *accObj, proto->GetRegisteredAccounts ())
			{
				IAccount *account = qobject_cast<IAccount*> (accObj);
				if (!account)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to cast"
							<< accObj
							<< "to IAccount for protocol"
							<< proto->GetProtocolID ();
					continue;
				}
				
				Ui_.AccountBox_->addItem (account->GetAccountName (),
						QVariant::fromValue<IAccount*> (account));
			}
		}
		
		if (Ui_.AccountBox_->count ())
			on_AccountBox__currentIndexChanged (0);
	}
	
	void BookmarksManagerDialog::on_AccountBox__currentIndexChanged (int index)
	{
		BMModel_->clear ();

		IAccount *account = Ui_.AccountBox_->itemData (index).value<IAccount*> ();
		IProtocol *proto = qobject_cast<IProtocol*> (account->GetParentProtocol ());
		IMUCJoinWidget *joiner = Proto2Joiner_ [proto->GetProtocolID ()];
		if (!joiner)
		{
			qWarning () << Q_FUNC_INFO
					<< "null joiner for"
					<< account->GetAccountID ()
					<< proto->GetProtocolID ();
			return;
		}

		const QByteArray& accId = account->GetAccountID ();
		Q_FOREACH (const QVariant& var, joiner->GetBookmarkedMUCs ())
		{
			const QVariantMap& map = var.toMap ();
			if (map.value ("AccountID").toByteArray () != accId)
				continue;

			const QString& name = map.value ("HumanReadableName").toString ();
			if (name.isEmpty ())
				continue;
			
			QStandardItem *item = new QStandardItem (name);
			item->setData (var);
			BMModel_->appendRow (item);
		}
	}
}
}
