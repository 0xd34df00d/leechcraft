/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "roomparticipantentry.h"
#include <QAction>
#include <QtDebug>
#include <gloox/mucroom.h>
#include "glooxaccount.h"
#include "roompublicmessage.h"
#include "glooxmessage.h"
#include "roomhandler.h"
#include "roomclentry.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
namespace Plugins
{
namespace Xoox
{
	RoomParticipantEntry::RoomParticipantEntry (const QString& nick,
			RoomHandler *rh, GlooxAccount *account)
	: EntryBase (account)
	, Nick_ (nick)
	, RoomHandler_ (rh)
	, Affiliation_ (gloox::AffiliationInvalid)
	, Role_ (gloox::RoleInvalid)
	{
	}

	QObject* RoomParticipantEntry::GetParentAccount () const
	{
		return Account_;
	}

	QObject* RoomParticipantEntry::GetParentCLEntry () const
	{
		return RoomHandler_->GetCLEntry ();
	}

	ICLEntry::Features RoomParticipantEntry::GetEntryFeatures () const
	{
		return FSessionEntry;
	}

	ICLEntry::EntryType RoomParticipantEntry::GetEntryType () const
	{
		return ETPrivateChat;
	}

	QString RoomParticipantEntry::GetEntryName () const
	{
		return Nick_;
	}

	void RoomParticipantEntry::SetEntryName (const QString& nick)
	{
		Nick_ = nick;
		emit nameChanged (Nick_);
	}

	QByteArray RoomParticipantEntry::GetEntryID () const
	{
		boost::shared_ptr<gloox::MUCRoom> room = RoomHandler_->GetCLEntry ()->GetRoom ();
		return (room->name () + "@" +
				room->service () + "/" +
				Nick_.toUtf8 ().constData ()).c_str ();
	}

	QStringList RoomParticipantEntry::Groups () const
	{
		boost::shared_ptr<gloox::MUCRoom> room = RoomHandler_->GetCLEntry ()->GetRoom ();
		QString roomName = QString::fromUtf8 (room->name ().c_str ()) +
				"@" +
				QString::fromUtf8 (room->service ().c_str ());
		return QStringList (tr ("%1 participants")
				.arg (roomName));
	}

	QStringList RoomParticipantEntry::Variants () const
	{
		return QStringList ("");
	}

	QObject* RoomParticipantEntry::CreateMessage (IMessage::MessageType type,
			const QString&, const QString& body)
	{
		GlooxMessage *msg = RoomHandler_->CreateMessage (type, Nick_, body);
		AllMessages_ << msg;
		return msg;
	}

	gloox::JID RoomParticipantEntry::GetJID () const
	{
		gloox::JID jid = RoomHandler_->GetRoomJID ();
		jid.setResource (Nick_.toUtf8 ().constData ());
		return jid;
	}

	gloox::MUCRoomAffiliation RoomParticipantEntry::GetAffiliation () const
	{
		return Affiliation_;
	}

	void RoomParticipantEntry::SetAffiliation (gloox::MUCRoomAffiliation aff)
	{
		Affiliation_ = aff;
	}

	gloox::MUCRoomRole RoomParticipantEntry::GetRole () const
	{
		return Role_;
	}

	void RoomParticipantEntry::SetRole (gloox::MUCRoomRole role)
	{
		Role_ = role;
	}
}
}
}
}
}
