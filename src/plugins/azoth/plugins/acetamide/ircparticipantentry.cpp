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
#include "ircmessage.h"
#include "ircaccount.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcParticipantEntry::IrcParticipantEntry (const QString& nick, IrcAccount *account)
	: EntryBase (account)
	, Nick_ (nick)
	{
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

}
}
}