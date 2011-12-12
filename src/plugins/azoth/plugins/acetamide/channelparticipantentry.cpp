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

#include "channelparticipantentry.h"
#include "channelhandler.h"
#include "channelclentry.h"
#include "ircmessage.h"
#include "ircaccount.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ChannelParticipantEntry::ChannelParticipantEntry (const QString& nick,
			ChannelHandler *ich, IrcAccount *acc)
	: IrcParticipantEntry (nick, acc)
	, ICH_ (ich)
	{
	}

	QObject* ChannelParticipantEntry::GetParentCLEntry () const
	{
		return ICH_->GetCLEntry ();
	}

	QString ChannelParticipantEntry::GetEntryID () const
	{
		return Account_->GetAccountName () + "/" +
				ICH_->GetChannelID () + "_" + Nick_;
	}

	QString ChannelParticipantEntry::GetHumanReadableID () const
	{
		return Nick_ + "_" + ICH_->GetChannelID ();
	}

	QStringList ChannelParticipantEntry::Groups () const
	{
		return QStringList () << ICH_->GetChannelID ();
	}

	void ChannelParticipantEntry::SetGroups (const QStringList&)
	{
	}

	QObject* ChannelParticipantEntry::CreateMessage (IMessage::MessageType type,
			const QString&, const QString& body)
	{
		IrcMessage *message = new IrcMessage (IMessage::MTChatMessage,
				IMessage::DOut,
				ICH_->GetChannelID (),
				Nick_,
				Account_->GetClientConnection ().get ());
		
		message->SetBody (body);
		message->SetDateTime (QDateTime::currentDateTime ());
		
		AllMessages_ << message;
		
		return message;
	}

	QList<ChannelRole> ChannelParticipantEntry::Roles () const
	{
		return Roles_;
	}

	ChannelRole ChannelParticipantEntry::HighestRole ()
	{
		if (Roles ().isEmpty ())
			return ChannelRole::Participant;

		qSort (Roles_.begin (), Roles_.end ());
		return Roles_.last ();
	}

	void ChannelParticipantEntry::SetRole (const ChannelRole& role)
	{
		if (!Roles_.contains (role))
			Roles_ << role;
	}

	void ChannelParticipantEntry::RemoveRole (const ChannelRole& role)
	{
		if (Roles_.contains (role))
			Roles_.removeAll (role);
	}

}
}
}