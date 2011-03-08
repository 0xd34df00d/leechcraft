/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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
#include <QAction>
#include <QtDebug>
#include "ircaccount.h"
#include "channelpublicmessage.h"
#include "channelhandler.h"
#include "channelclentry.h"
#include "ircmessage.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ChannelParticipantEntry::ChannelParticipantEntry (const QString& nick, 
			ChannelHandler *ch, IrcAccount *account)
	: Account_ (account)
	, NickName_ (nick)
	, ChannelHandler_ (ch)
	{
	}

	QObject* ChannelParticipantEntry::GetParentAccount () const
	{
		return Account_;
	}

	QObject* ChannelParticipantEntry::GetParentCLEntry () const
	{
		return ChannelHandler_->GetCLEntry ();
	}

	ICLEntry::Features ChannelParticipantEntry::GetEntryFeatures () const
	{
		return FSessionEntry;
	}
	
	ICLEntry::EntryType ChannelParticipantEntry::GetEntryType () const
	{
		return ETPrivateChat;
	}

	QString ChannelParticipantEntry::GetEntryName () const
	{
		return NickName_;
	}

	void ChannelParticipantEntry::SetEntryName (const QString& nick)
	{
		NickName_ = nick;
		emit nameChanged (NickName_);
	}
	
	QString ChannelParticipantEntry::GetEntryID () const
	{
		return ChannelHandler_->GetChannelID () + "/" + NickName_;
	}

	QStringList ChannelParticipantEntry::Groups () const
	{
		return QStringList (tr ("%1 participants")
				.arg (ChannelHandler_->GetChannelID ()));
	}

	void ChannelParticipantEntry::SetGroups (const QStringList&)
	{
	}

	QStringList ChannelParticipantEntry::Variants () const
	{
		return QStringList ("");
	}

	QObject* ChannelParticipantEntry::CreateMessage (IMessage::MessageType type,
			const QString& , const QString& body)
	{
		IrcMessage *msg = ChannelHandler_->CreateMessage (type, NickName_, body);
		AllMessages_ << msg;
		return msg;
	}

	void ChannelParticipantEntry::HandleMessage (IrcMessage *msg)
	{
		AllMessages_ << msg;
		emit gotMessage (msg);
	}


	QString ChannelParticipantEntry::GetChannelID () const
	{
		return ChannelHandler_->GetChannelID () + "/" + NickName_;
	}

	QString ChannelParticipantEntry::GetNick () const
	{
		return NickName_;
	}
	
	QObject* ChannelParticipantEntry::GetObject ()
	{
		return this;
	}

	QList<QObject*> ChannelParticipantEntry::GetAllMessages () const
	{
		return QList<QObject*> ();
	}

	EntryStatus ChannelParticipantEntry::GetStatus (const QString& variant) const
	{
		return EntryStatus (SOnline, variant);
	}

	QImage ChannelParticipantEntry::GetAvatar () const
	{
		return QImage ();
	}

	QString ChannelParticipantEntry::GetRawInfo () const
	{
		return QString ();
	}

	void ChannelParticipantEntry::ShowInfo ()
	{
	}

	QList<QAction*> ChannelParticipantEntry::GetActions () const
	{
		return QList<QAction*> ();
	}

	QMap<QString, QVariant> ChannelParticipantEntry::GetClientInfo (const QString&) const
	{
		return QMap<QString, QVariant> ();
	}
};
};
};























