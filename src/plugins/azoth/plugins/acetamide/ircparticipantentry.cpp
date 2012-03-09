/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "ircparticipantentry.h"
#include <QAction>
#include "ircmessage.h"
#include "ircaccount.h"
#include "clientconnection.h"

namespace LeechCraft
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

	QObject* IrcParticipantEntry::GetParentAccount () const
	{
		return Account_;
	}

	ICLEntry::Features IrcParticipantEntry::GetEntryFeatures () const
	{
		return FSessionEntry;
	}

	ICLEntry::EntryType IrcParticipantEntry::GetEntryType () const
	{
		return ETPrivateChat;
	}

	QString IrcParticipantEntry::GetEntryName () const
	{
		return Nick_;
	}

	void IrcParticipantEntry::SetEntryName (const QString& name)
	{
		Nick_ = name;

		Q_FOREACH (QObject *message, AllMessages_)
		{
			IrcMessage *msg = qobject_cast<IrcMessage*> (message);
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