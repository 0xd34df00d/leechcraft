/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "roomparticipantentry.h"
#include <QAction>
#include <QtDebug>
#include <QXmppMucManager.h>
#include "glooxaccount.h"
#include "roompublicmessage.h"
#include "glooxmessage.h"
#include "roomhandler.h"
#include "roomclentry.h"
#include "core.h"
#include "avatarsstorage.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	RoomParticipantEntry::RoomParticipantEntry (const QString& nick,
			RoomHandler *rh, GlooxAccount *account)
	: EntryBase (rh->GetRoomJID () + "/" + nick, account)
	, Nick_ (nick)
	, RoomHandler_ (rh)
	, Affiliation_ (QXmppMucItem::UnspecifiedAffiliation)
	, Role_ (QXmppMucItem::UnspecifiedRole)
	{
	}

	ICLEntry* RoomParticipantEntry::GetParentCLEntry () const
	{
		return RoomHandler_->GetCLEntry ();
	}

	ICLEntry::Features RoomParticipantEntry::GetEntryFeatures () const
	{
		return FSessionEntry;
	}

	ICLEntry::EntryType RoomParticipantEntry::GetEntryType () const
	{
		return EntryType::PrivateChat;
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
		return Account_->GetAccountID () + '_' + HumanReadableId_;
	}

	QStringList RoomParticipantEntry::Groups () const
	{
		return QStringList (RoomHandler_->GetCLEntry ()->GetGroupName ());
	}

	void RoomParticipantEntry::SetGroups (const QStringList&)
	{
	}

	QStringList RoomParticipantEntry::Variants () const
	{
		return { {} };
	}

	IMessage* RoomParticipantEntry::CreateMessage (IMessage::Type type,
			const QString&, const QString& body)
	{
		const auto msg = RoomHandler_->CreateMessage (type, Nick_, body);
		AllMessages_ << msg;
		return msg;
	}

	QString RoomParticipantEntry::GetJID () const
	{
		return RoomHandler_->GetRoomJID () + "/" + Nick_;
	}

	QString RoomParticipantEntry::GetRealJID () const
	{
		return RoomHandler_->GetRoom ()->
				participantPresence (GetJID ()).mucItem ().jid ();
	}

	QString RoomParticipantEntry::GetNick () const
	{
		return Nick_;
	}

	namespace
	{
		template<typename T>
		void MergeMessages (QList<T*>& ourMessages, const QList<T*>& otherMessages)
		{
			const auto size = ourMessages.size ();
			ourMessages += otherMessages;
			std::inplace_merge (ourMessages.begin (),
					ourMessages.begin () + size,
					ourMessages.end (),
					[] (T *msg1, T *msg2) { return msg1->GetDateTime () < msg2->GetDateTime (); });
		}
	}

	void RoomParticipantEntry::StealMessagesFrom (RoomParticipantEntry *other)
	{
		if (other->AllMessages_.isEmpty ())
			return;

		for (auto msg : other->AllMessages_)
			msg->SetVariant (Nick_);

		MergeMessages (AllMessages_, other->AllMessages_);
		other->AllMessages_.clear ();

		if (other->HasUnreadMsgs ())
		{
			for (auto msg : other->UnreadMessages_)
				emit gotMessage (msg);

			MergeMessages (UnreadMessages_, other->UnreadMessages_);
		}
	}

	QXmppMucItem::Affiliation RoomParticipantEntry::GetAffiliation () const
	{
		return Affiliation_;
	}

	void RoomParticipantEntry::SetAffiliation (QXmppMucItem::Affiliation aff)
	{
		Affiliation_ = aff;
		emit permsChanged ();
	}

	QXmppMucItem::Role RoomParticipantEntry::GetRole () const
	{
		return Role_;
	}

	void RoomParticipantEntry::SetRole (QXmppMucItem::Role role)
	{
		Role_ = role;
		emit permsChanged ();
	}
}
}
}
