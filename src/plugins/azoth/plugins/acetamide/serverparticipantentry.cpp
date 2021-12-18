/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "serverparticipantentry.h"
#include <QAction>
#include "clientconnection.h"
#include "ircaccount.h"
#include "ircserverclentry.h"
#include "ircmessage.h"
#include "ircserverhandler.h"

namespace LC::Azoth::Acetamide
{
	ServerParticipantEntry::ServerParticipantEntry (QString nick,
			IrcServerHandler *ish, IrcAccount *acc)
	: IrcParticipantEntry { std::move (nick), acc }
	, ISH_ { ish }
	{
		ServerID_ = ish->GetServerID ();
	}

	ICLEntry* ServerParticipantEntry::GetParentCLEntry () const
	{
		return ISH_->GetCLEntry ();
	}

	QString ServerParticipantEntry::GetEntryID () const
	{
		return Account_->GetAccountName () + "/" +
				ISH_->GetServerID () + "_" + Nick_;
	}

	QString ServerParticipantEntry::GetHumanReadableID () const
	{
		return Nick_ + "_" + ISH_->GetServerID ();
	}

	QStringList ServerParticipantEntry::Groups () const
	{
		return QStringList (tr ("Private chats"));
	}

	void ServerParticipantEntry::SetGroups (const QStringList&)
	{
	}

	IMessage* ServerParticipantEntry::CreateMessage (IMessage::Type,
			const QString&, const QString& body)
	{
 		const auto message = new IrcMessage (IMessage::Type::ChatMessage,
				IMessage::Direction::Out,
				ISH_->GetServerID (),
				Nick_,
				Account_->GetClientConnection ().get ());

		message->SetBody (body);
		message->SetDateTime (QDateTime::currentDateTime ());

		return message;
	}
}
