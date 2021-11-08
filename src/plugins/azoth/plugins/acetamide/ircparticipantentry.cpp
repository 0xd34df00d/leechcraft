/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ircparticipantentry.h"
#include <QAction>
#include "ircmessage.h"
#include "ircaccount.h"
#include "clientconnection.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	IrcParticipantEntry::IrcParticipantEntry (const QString& nick, IrcAccount *account)
	: EntryBase (account)
	, Nick_ (nick)
	, IsPrivateChat_ (false)
	{
		QAction *closePrivate = new QAction ("Close chat", this);

		connect (closePrivate,
				SIGNAL (triggered ()),
				this,
				SLOT (handleClosePrivate ()));

		Actions_ << closePrivate;
	}

	IAccount* IrcParticipantEntry::GetParentAccount () const
	{
		return Account_;
	}

	ICLEntry::Features IrcParticipantEntry::GetEntryFeatures () const
	{
		return FSessionEntry;
	}

	ICLEntry::EntryType IrcParticipantEntry::GetEntryType () const
	{
		return EntryType::PrivateChat;
	}

	QString IrcParticipantEntry::GetEntryName () const
	{
		return Nick_;
	}

	void IrcParticipantEntry::SetEntryName (const QString& name)
	{
		Nick_ = name;
		emit nameChanged (Nick_);

		for (const auto message : AllMessages_)
		{
			const auto msg = dynamic_cast<IrcMessage*> (message);
			if (!msg)
			{
				qWarning () << Q_FUNC_INFO
						<< "is not an object of IrcMessage"
						<< message;
				continue;
			}

			msg->SetOtherVariant (name);
		}
	}

	QStringList IrcParticipantEntry::Variants () const
	{
		return QStringList (QString ());
	}

	QString IrcParticipantEntry::GetUserName () const
	{
		return UserName_;
	}

	void IrcParticipantEntry::SetUserName (const QString& user)
	{
		UserName_ = user;
	}

	QString IrcParticipantEntry::GetHostName () const
	{
		return HostName_;
	}

	void IrcParticipantEntry::SetHostName (const QString& host)
	{
		HostName_ = host;
	}

	QString IrcParticipantEntry::GetRealName () const
	{
		return RealName_;
	}

	void IrcParticipantEntry::SetRealName (const QString& realName)
	{
		RealName_ = realName;
	}

	QString IrcParticipantEntry::GetServerID () const
	{
		return ServerID_;
	}

	bool IrcParticipantEntry::IsPrivateChat () const
	{
		return IsPrivateChat_;
	}

	void IrcParticipantEntry::SetPrivateChat (bool isPrivate)
	{
		IsPrivateChat_ = isPrivate;
	}

	void IrcParticipantEntry::handleClosePrivate ()
	{
		IsPrivateChat_ = false;
		Account_->GetClientConnection ()->ClosePrivateChat (ServerID_, Nick_);
	}

}
}
}
