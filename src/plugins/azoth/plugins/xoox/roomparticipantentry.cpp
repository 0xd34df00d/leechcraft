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
#include "glooxaccount.h"
#include "roompublicmessage.h"
#include "glooxmessage.h"
#include "roomhandler.h"
#include "roomclentry.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	RoomParticipantEntry::RoomParticipantEntry (const QString& nick,
			RoomHandler *rh, GlooxAccount *account)
	: EntryBase (account)
	, Nick_ (nick)
	, RoomHandler_ (rh)
	, Affiliation_ (QXmppMucAdminIq::Item::UnspecifiedAffiliation)
	, Role_ (QXmppMucAdminIq::Item::UnspecifiedRole)
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

	QString RoomParticipantEntry::GetEntryID () const
	{
		return RoomHandler_->GetRoomJID () + "/" + Nick_;
	}

	QStringList RoomParticipantEntry::Groups () const
	{
		return QStringList (tr ("%1 participants")
				.arg (RoomHandler_->GetRoomJID ()));
	}

	void RoomParticipantEntry::SetGroups (const QStringList&)
	{
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

	QString RoomParticipantEntry::GetJID () const
	{
		return RoomHandler_->GetRoomJID () + "/" + Nick_;
	}

	QString RoomParticipantEntry::GetNick () const
	{
		return Nick_;
	}

	QXmppMucAdminIq::Item::Affiliation RoomParticipantEntry::GetAffiliation () const
	{
		return Affiliation_;
	}

	void RoomParticipantEntry::SetAffiliation (QXmppMucAdminIq::Item::Affiliation aff)
	{
		Affiliation_ = aff;
	}

	QXmppMucAdminIq::Item::Role RoomParticipantEntry::GetRole () const
	{
		return Role_;
	}

	void RoomParticipantEntry::SetRole (QXmppMucAdminIq::Item::Role role)
	{
		Role_ = role;
	}
}
}
}
