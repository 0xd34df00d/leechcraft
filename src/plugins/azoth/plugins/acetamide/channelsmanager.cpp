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

#include "channelsmanager.h"
#include "xmlsettingsmanager.h"
#include "ircserverhandler.h"
#include "channelhandler.h"
#include "channelclentry.h"
#include "ircaccount.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ChannelsManager::ChannelsManager (IrcServerHandler *ish)
	: QObject (ish)
	, ISH_ (ish)
	{
	}

	IrcAccount* ChannelsManager::GetAccount () const
	{
		return ISH_->GetAccount ();
	}

	QString ChannelsManager::GetOurNick () const
	{
		return ISH_->GetNickName ();
	}

	QString ChannelsManager::GetServerID () const
	{
		return ISH_->GetServerID ();
	}

	ServerOptions ChannelsManager::GetServerOptions () const
	{
		return ISH_->GetServerOptions ();
	}

	QObjectList ChannelsManager::GetCLEntries () const
	{
		QObjectList result;
		Q_FOREACH (auto ich, ChannelHandlers_.values ())
		{
			result << ich->GetParticipants ();
			result << ich->GetCLEntry ();
		}

		return result;
	}

	ChannelHandler* ChannelsManager::GetChannelHandler (const QString& channel)
	{
		return ChannelHandlers_.value (channel.toLower ()).get ();
	}

	QList<std::shared_ptr<ChannelHandler>> ChannelsManager::GetChannels () const
	{
		return ChannelHandlers_.values ();
	}

	bool ChannelsManager::IsChannelExists (const QString& channel) const
	{
		return ChannelHandlers_.contains (channel.toLower ());
	}

	int ChannelsManager::Count () const
	{
		return ChannelHandlers_.count ();
	}

	QSet<ChannelOptions> ChannelsManager::GetChannelsQueue () const
	{
		return ChannelsQueue_;
	}

	void ChannelsManager::CleanQueue ()
	{
		ChannelsQueue_.clear ();
	}

	void ChannelsManager::AddChannel2Queue (const ChannelOptions& options)
	{
		ChannelsQueue_ << options;
	}

	bool ChannelsManager::AddChannel (const ChannelOptions& options)
	{
		std::shared_ptr<ChannelHandler> ch (new ChannelHandler (options, this));
		ChannelHandlers_ [options.ChannelName_.toLower ()] = ch;

		ChannelCLEntry *ichEntry = ch->GetCLEntry ();
		if (!ichEntry)
			return false;

		return true;
	}

	void ChannelsManager::LeaveChannel (const QString& channel, const QString& msg)
	{
		ISH_->LeaveChannel (channel, msg);
	}

	void ChannelsManager::CloseChannel (const QString& channel)
	{
		const QString& chnnl = channel.toLower ();
		if (ChannelHandlers_.contains (chnnl))
			ChannelHandlers_ [chnnl]->CloseChannel ();
	}

	void ChannelsManager::CloseAllChannels ()
	{
		Q_FOREACH (auto ich, ChannelHandlers_.values ())
			ich->CloseChannel ();
	}

	void ChannelsManager::UnregisterChannel (ChannelHandler *ich)
	{
		ChannelHandlers_.remove (ich->GetChannelOptions ().ChannelName_);

		if (!ChannelHandlers_.count () &&
				XmlSettingsManager::Instance ()
						.property ("AutoDisconnectFromServer").toBool ())
			ISH_->DisconnectFromServer ();
	}

	QHash<QString, QObject*> ChannelsManager::GetParticipantsByNick (const QString& nick) const
	{
		QHash<QString, QObject*> result;
		Q_FOREACH (auto ich, ChannelHandlers_.values ())
			if (ich->IsUserExists (nick))
				result [ich->GetChannelOptions ().ChannelName_] = ich->GetParticipantEntry (nick).get ();
		return result;
	}

	void ChannelsManager::AddParticipant (const QString& channel, const QString& nick,
			const QString& user, const QString& host)
	{
		const QString& chnnl = channel.toLower ();
		if (ChannelHandlers_.contains (chnnl))
			ChannelHandlers_ [chnnl]->SetChannelUser (nick, user, host);
	}

	void ChannelsManager::LeaveParticipant (const QString& channel,
			const QString& nick, const QString& msg)
	{
		const QString& chnnl = channel.toLower ();
		if (ChannelHandlers_.contains (chnnl))
			ChannelHandlers_ [chnnl]->LeaveParticipant (nick, msg);
	}

	void ChannelsManager::QuitParticipant (const QString& nick, const QString& msg)
	{
		Q_FOREACH (auto ch, ChannelHandlers_)
			if (ch->IsUserExists (nick))
				ch->LeaveParticipant (nick, msg);
	}

	void ChannelsManager::KickParticipant (const QString& channel,
			const QString& target, const QString& reason, const QString& who)
	{
		const QString& chnnl = channel.toLower ();
		if (ChannelHandlers_.contains (chnnl))
			ChannelHandlers_ [chnnl]->KickParticipant (target, reason, who);
	}

	void ChannelsManager::KickCommand (const QString& channel,
			const QString& nick, const QString& reason)
	{
		ISH_->KickParticipant (channel, nick, reason);
	}

	void ChannelsManager::ChangeNickname (const QString& oldNick, const QString& newNick)
	{
		Q_FOREACH (auto ich, ChannelHandlers_.values ())
			if (ich->IsUserExists (oldNick))
				ich->ChangeNickname (oldNick, newNick);
	}

	void ChannelsManager::GotNames (const QString& channel, const QStringList& participants)
	{
		if (IsChannelExists (channel) &&
				!ChannelHandlers_ [channel]->IsRosterReceived ())
		{
			Q_FOREACH (const QString& nick, participants)
			{
				if (!nick.isEmpty ())
					ChannelHandlers_ [channel]->SetChannelUser (nick);
			}
		}
		else
			ReceiveCmdAnswerMessage ("names", participants.join (" "), false);
	}

	void ChannelsManager::GotEndOfNamesCmd (const QString& channel)
	{
		if (ChannelHandlers_.contains (channel) &&
				!ChannelHandlers_ [channel]->IsRosterReceived ())
		{
			ChannelHandlers_ [channel]->SetRosterReceived (true);
			ISH_->GetAccount ()->handleGotRosterItems (QObjectList () << ChannelHandlers_ [channel]->GetCLEntry ());
		}
		else
			ReceiveCmdAnswerMessage ("names", "End of /NAMES", true);
	}

	void ChannelsManager::SendPublicMessage (const QString& channel, const QString& msg)
	{
		LastActiveChannel_ = channel.toLower ();
		const QString& chnnl = channel.toLower ();
		if (msg.startsWith ('/'))
		{
			if (ChannelHandlers_.contains (chnnl))
			{
				const QString& cmd = ISH_->ParseMessageForCommand (msg, chnnl);
				if (cmd == "say")
					ChannelHandlers_ [chnnl]->HandleIncomingMessage (ISH_->GetNickName (),
							msg.mid (4));
				else if (cmd == "me")
					ChannelHandlers_ [chnnl]->HandleIncomingMessage (ISH_->GetNickName (),
							msg);
				else
					ChannelHandlers_ [chnnl]->HandleServiceMessage (msg,
							IMessage::MTEventMessage,
							IMessage::MSTOther);
			}
		}
		else
		{
			ISH_->SendPublicMessage (msg, chnnl);
			if (ChannelHandlers_.contains (chnnl))
				ChannelHandlers_ [chnnl]->HandleIncomingMessage (ISH_->GetNickName (),
						msg);
		}
	}

	void ChannelsManager::ReceivePublicMessage (const QString& channel,
			const QString& nick, const QString& msg)
	{
		const QString& chnnl = channel.toLower ();
		if (ChannelHandlers_.contains (chnnl))
			ChannelHandlers_ [chnnl]->HandleIncomingMessage (nick, msg);
	}

	bool ChannelsManager::ReceiveCmdAnswerMessage (const QString& cmd,
			const QString& answer, bool endOfCmd)
	{
		if (LastActiveChannel_.isEmpty () ||
				!ChannelHandlers_.contains (LastActiveChannel_))
			return false;

		ChannelHandlers_ [LastActiveChannel_]->HandleServiceMessage (answer,
				IMessage::MTEventMessage,
				IMessage::MSTOther);

		return true;
	}

	void ChannelsManager::SetMUCSubject (const QString& channel, const QString& topic)
	{
		const QString& chnnl = channel.toLower ();
		if (ChannelHandlers_.contains (chnnl))
			ChannelHandlers_ [chnnl]->SetMUCSubject (topic);

		ReceiveCmdAnswerMessage ("topic", topic, ISH_->IsCmdHasLongAnswer ("topic"));
	}

	void ChannelsManager::SetTopic (const QString& channel, const QString& topic)
	{
		ISH_->SetTopic (channel, topic);
	}

	void ChannelsManager::CTCPReply (const QString& msg)
	{
		Q_FOREACH (auto ich, ChannelHandlers_.values ())
			ich->HandleServiceMessage (msg,
					IMessage::MTServiceMessage,
					IMessage::MSTOther);
	}

	void ChannelsManager::CTCPRequestResult (const QString& msg)
	{
		Q_FOREACH (auto ich, ChannelHandlers_.values ())
		{
			ich->HandleServiceMessage (msg,
					IMessage::MTServiceMessage,
					IMessage::MSTOther);
		}
	}

	void ChannelsManager::SetBanListItem (const QString& channel,
			const QString& mask, const QString& nick, const QDateTime& time)
	{
		if (ChannelHandlers_.contains (channel))
			ChannelHandlers_ [channel]->SetBanListItem (mask, nick, time);
	}

	void ChannelsManager::RequestBanList (const QString& channel)
	{
		ISH_->GetBanList (channel);
	}

	void ChannelsManager::AddBanListItem (const QString& channel, const QString& mask)
	{
		ISH_->AddBanListItem (channel, mask);
	}

	void ChannelsManager::RemoveBanListItem (const QString& channel, const QString& mask)
	{
		ISH_->RemoveBanListItem (channel, mask);
	}

	void ChannelsManager::SetExceptListItem (const QString& channel,
			const QString& mask, const QString& nick, const QDateTime& time)
	{
		if (ChannelHandlers_.contains (channel))
			ChannelHandlers_ [channel]->SetExceptListItem (mask, nick, time);
	}

	void ChannelsManager::RequestExceptList (const QString& channel)
	{
		ISH_->GetExceptList (channel);
	}

	void ChannelsManager::AddExceptListItem (const QString& channel, const QString& mask)
	{
		ISH_->AddExceptListItem (channel, mask);
	}

	void ChannelsManager::RemoveExceptListItem (const QString& channel, const QString& mask)
	{
		ISH_->RemoveExceptListItem (channel, mask);
	}

	void ChannelsManager::SetInviteListItem (const QString& channel,
			const QString& mask, const QString& nick, const QDateTime& time)
	{
		if (ChannelHandlers_.contains (channel))
			ChannelHandlers_ [channel]->SetInviteListItem (mask, nick, time);
	}

	void ChannelsManager::RequestInviteList (const QString& channel)
	{
		ISH_->GetInviteList (channel);
	}

	void ChannelsManager::AddInviteListItem (const QString& channel, const QString& mask)
	{
		ISH_->AddInviteListItem (channel, mask);
	}

	void ChannelsManager::RemoveInviteListItem (const QString& channel, const QString& mask)
	{
		ISH_->RemoveInviteListItem (channel, mask);
	}

	void ChannelsManager::ParseChanMode (const QString& channel,
			const QString& mode, const QString& value)
	{
		bool action = mode [0] == '+';

		for (int i = 1; i < mode.length (); ++i)
		{
			switch (mode [i].toAscii ())
			{
				case 'o':
					if (!value.isEmpty () &&
							ChannelHandlers_ [channel]->IsUserExists (value))
					{
						ChannelParticipantEntry_ptr entry =
								ChannelHandlers_ [channel]->GetParticipantEntry (value);
						if (action)
							entry->SetRole (ChannelRole::Operator);
						else
							entry->RemoveRole (ChannelRole::Operator);

						ChannelHandlers_ [channel]->MakePermsChangedMessage (value,
								ChannelRole::Operator, action);
					}
					break;
				case 'v':
					if (!value.isEmpty () &&
							ChannelHandlers_ [channel]->IsUserExists (value))
					{
						ChannelParticipantEntry_ptr entry =
								ChannelHandlers_ [channel]->GetParticipantEntry (value);
						if (action)
							entry->SetRole (ChannelRole::Voiced);
						else
							entry->RemoveRole (ChannelRole::Voiced);

						ChannelHandlers_ [channel]->MakePermsChangedMessage (value,
								ChannelRole::Voiced, action);
					}
					break;
				case 'a':
					// may be it is nessesary
					break;
				case 'i':
					ChannelHandlers_ [channel]->SetInviteMode (action);
					break;
				case 'm':
					ChannelHandlers_ [channel]->SetModerateMode (action);
					break;
				case 'n':
					ChannelHandlers_ [channel]->SetBlockOutsideMessagesMode (action);
					break;
				case 'q':
					if (ISH_->GetISupport ().contains ("PREFIX") &&
							ISH_->GetISupport () ["PREFIX"].contains ('q'))
						if (!value.isEmpty () &&
								ChannelHandlers_ [channel]->IsUserExists (value))
						{
							ChannelParticipantEntry_ptr entry =
									ChannelHandlers_ [channel]->GetParticipantEntry (value);
							if (action)
								entry->SetRole (ChannelRole::Owner);
							else
								entry->RemoveRole (ChannelRole::Owner);

							ChannelHandlers_ [channel]->MakePermsChangedMessage (value,
									ChannelRole::Owner, action);
						}
						break;
				case 'p':
					ChannelHandlers_ [channel]->SetPrivateMode (action);
					break;
				case 'r':
					ChannelHandlers_ [channel]->SetServerReOpMode (action);
					break;
				case 's':
					ChannelHandlers_ [channel]->SetSecretMode (action);
					break;
				case 't':
					ChannelHandlers_ [channel]->SetOnlyOpTopicChangeMode (action);
					break;
				case 'l':
					ChannelHandlers_ [channel]->
							SetUserLimit (action, value.toInt ());
					break;
				case 'k':
					ChannelHandlers_ [channel]->SetChannelKey (action, value);
					break;
				case 'b':
					ISH_->ShowAnswer ("mode", tr ("%1 added to your ban list.")
							.arg (value));
					break;
				case 'e':
					ISH_->ShowAnswer ("mode", tr ("%1 added to your except list.")
							.arg (value));
					break;
				case 'I':
					ISH_->ShowAnswer ("mode", tr ("%1 added to your invite list.")
							.arg (value));
					break;
			}
		}
	}

	void ChannelsManager::SetNewChannelMode (const QString& channel,
			const QString& mode, const QString& name)
	{
		ISH_->SetNewChannelMode (channel, mode, name);
	}

	void ChannelsManager::SetNewChannelModes (const QString& channel,
			const ChannelModes& modes)
	{
		ISH_->SetNewChannelModes (channel, modes);
	}

	void ChannelsManager::RequestWhoIs (const QString& channel, const QString& nick)
	{
		LastActiveChannel_ = channel;
		ISH_->RequestWhoIs (nick);
	}

	void ChannelsManager::RequestWhoWas (const QString& channel, const QString& nick)
	{
		LastActiveChannel_ = channel;
		ISH_->RequestWhoWas (nick);
	}

	void ChannelsManager::RequestWho (const QString& channel, const QString& nick)
	{
		LastActiveChannel_ = channel;
		ISH_->RequestWho (nick);
	}

	void ChannelsManager::CTCPRequest (const QStringList& cmd, const QString& channel)
	{
		LastActiveChannel_ = channel;
		ISH_->CTCPRequst (cmd);
	}

	QMap<QString, QString> ChannelsManager::GetISupport () const
	{
		return ISH_->GetISupport ();
	}

	void ChannelsManager::SetPrivateChat (const QString& nick)
	{
		Q_FOREACH (QObject *entryObj, GetParticipantsByNick (nick).values ())
		{
			IrcParticipantEntry *entry = qobject_cast<IrcParticipantEntry*> (entryObj);
			if (!entry)
				continue;

			entry->SetPrivateChat (true);
		}
	}

	void ChannelsManager::CreateServerParticipantEntry (QString nick)
	{
		ISH_->CreateServerParticipantEntry (nick);
	}

	void ChannelsManager::UpdateEntry (const WhoMessage& message)
	{
		if (!ChannelHandlers_.contains (message.Channel_.toLower ()))
			return;

		ChannelHandlers_ [message.Channel_.toLower ()]->UpdateEntry (message);
	}

	int ChannelsManager::GetChannelUsersCount (const QString& channel)
	{
		if (!ChannelHandlers_.contains (channel.toLower ()))
			return 0;

		return ChannelHandlers_ [channel.toLower ()]->GetParticipants ().count ();
	}

	void ChannelsManager::ClosePrivateChat (const QString& nick)
	{
		ISH_->ClosePrivateChat (nick);
	}

	uint qHash (const ChannelOptions& opts)
	{
		return qHash (opts.ChannelName_ + opts.ChannelPassword_ + opts.ServerName_);
	}
}
}
}