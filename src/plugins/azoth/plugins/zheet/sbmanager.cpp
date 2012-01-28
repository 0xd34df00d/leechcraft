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

#include "sbmanager.h"
#include <QFileInfo>
#include <QtDebug>
#include "msnaccount.h"
#include "msnmessage.h"
#include "msnbuddyentry.h"
#include "callbacks.h"
#include "zheetutil.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	SBManager::SBManager (Callbacks *cb, MSNAccount *acc)
	: QObject (acc)
	, Account_ (acc)
	, CB_ (cb)
	{
		connect (CB_,
				SIGNAL (gotSB (MSN::SwitchboardServerConnection*, const MSNBuddyEntry*)),
				this,
				SLOT (handleGotSB (MSN::SwitchboardServerConnection*, const MSNBuddyEntry*)));
		connect (CB_,
				SIGNAL (buddyJoinedSB (MSN::SwitchboardServerConnection*, const MSNBuddyEntry*)),
				this,
				SLOT (handleBuddyJoined (MSN::SwitchboardServerConnection*, const MSNBuddyEntry*)));
		connect (CB_,
				SIGNAL (buddyLeftSB (MSN::SwitchboardServerConnection*,const MSNBuddyEntry*)),
				this,
				SLOT (handleBuddyLeft (MSN::SwitchboardServerConnection*, const MSNBuddyEntry*)));
		connect (CB_,
				SIGNAL (messageDelivered (int)),
				this,
				SLOT (handleMessageDelivered (int)));
	}

	void SBManager::SendMessage (MSNMessage *msg, const MSNBuddyEntry *entry)
	{
		if (Switchboards_.contains (entry))
		{
			const int id = Switchboards_ [entry]->
					sendMessage (ZheetUtil::ToStd (msg->GetBody ()));
			msg->SetID (id);
			PendingDelivery_ [id] = msg;
			return;
		}

		PendingMessages_ [entry] << msg;
		Account_->GetNSConnection ()->requestSwitchboardConnection (entry);
	}

	void SBManager::SendNudge (const QString&, const MSNBuddyEntry *entry)
	{
		if (Switchboards_.contains (entry))
		{
			Switchboards_ [entry]->sendNudge ();
			return;
		}

		PendingNudges_ << entry;
		Account_->GetNSConnection ()->requestSwitchboardConnection (entry);
	}
	
	void SBManager::SendFile (const QString& file, uint id, const MSNBuddyEntry *entry)
	{
		const QFileInfo info (file);
		MSN::fileTransferInvite ft = { 1,
				id,
				ZheetUtil::ToStd (entry->GetHumanReadableID ()),
				ZheetUtil::ToStd (file),
				ZheetUtil::ToStd (info.fileName ()),
				std::string (),
				info.size () };
		if (Switchboards_.contains (entry))
		{
			Switchboards_ [entry]->sendFile (ft);
			return;
		}

		PendingTransfers_ [entry] << ft;
		Account_->GetNSConnection ()->requestSwitchboardConnection (entry);
	}

	void SBManager::handleGotSB (MSN::SwitchboardServerConnection *conn, const MSNBuddyEntry *entry)
	{
		conn->inviteUser (ZheetUtil::ToStd (entry->GetHumanReadableID ()));
	}

	void SBManager::handleBuddyJoined (MSN::SwitchboardServerConnection *conn, const MSNBuddyEntry *entry)
	{
		Switchboards_ [entry] = conn;

		Q_FOREACH (MSNMessage *msg, PendingMessages_.take (entry))
			SendMessage (msg, entry);
			
		Q_FOREACH (auto ft, PendingTransfers_.take (entry))
			conn->sendFile (ft);

		if (PendingNudges_.remove (entry))
			SendNudge (QString (), entry);
	}

	void SBManager::handleBuddyLeft (MSN::SwitchboardServerConnection *conn, const MSNBuddyEntry *entry)
	{
		Switchboards_.remove (entry);
	}

	void SBManager::handleMessageDelivered (int id)
	{
		if (!PendingDelivery_.contains (id))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown message ID"
					<< id
					<< PendingDelivery_;
			return;
		}

		PendingDelivery_.take (id)->SetDelivered ();
	}
}
}
}
