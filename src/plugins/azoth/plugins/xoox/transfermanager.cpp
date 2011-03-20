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

#include "transfermanager.h"
#include <QXmppTransferManager.h>
#include "transferjob.h"
#include "glooxaccount.h"
#include "clientconnection.h"
#include "glooxclentry.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	TransferManager::TransferManager (QXmppTransferManager *manager, GlooxAccount *account)
	: QObject (manager)
	, Manager_ (manager)
	, Account_ (account)
	{
		connect (Manager_,
				SIGNAL (fileReceived (QXmppTransferJob*)),
				this,
				SLOT (handleFileReceived (QXmppTransferJob*)));
	}

	QObject* TransferManager::SendFile (const QString& id,
			const QString& sourceVar, const QString& name)
	{
		QString target = GlooxCLEntry::JIDFromID (Account_, id);
		QString var = sourceVar;
		if (var.isEmpty ())
		{
			QObject *entryObj = Account_->GetClientConnection ()->
					GetCLEntry (target, QString ());
			GlooxCLEntry *entry = qobject_cast<GlooxCLEntry*> (entryObj);
			var = entry->Variants ().value (0);
		}
		target += '/' + var;
		return new TransferJob (Manager_->sendFile (target, name), this);
	}
	
	GlooxAccount* TransferManager::GetAccount () const
	{
		return Account_;
	}

	void TransferManager::handleFileReceived (QXmppTransferJob *job)
	{
		emit fileOffered (new TransferJob (job, this));
	}
}
}
}
