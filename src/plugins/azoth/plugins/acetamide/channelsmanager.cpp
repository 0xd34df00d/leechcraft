/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "channelsmanager.h"
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include "xmlsettingsmanager.h"
#include "ircserverhandler.h"
#include "channelhandler.h"
#include "channelclentry.h"
#include "ircaccount.h"

namespace LC::Azoth::Acetamide
{
	ChannelsManager::ChannelsManager (IrcServerHandler *ish)
	: QObject { ish }
	, ISH_ { ish }
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
		for (const auto& ich : ChannelHandlers_)
		{
			result << ich->GetCLEntry ();
			result << ich->GetParticipants ();
		}

		return result;
	}

	ChannelHandler* ChannelsManager::GetChannelHandler (const QString& channel)
	{
		return ChannelHandlers_.value (channel.toLower ()).get ();
	}

	QList<ChannelHandler*> ChannelsManager::GetChannels () const
	{
		return Util::Map (ChannelHandlers_, &ChannelHandler_ptr::get);
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
		const auto ch = std::make_shared<ChannelHandler> (options, this);
		ChannelHandlers_ [options.ChannelName_.toLower ()] = ch;
		return ch->GetCLEntry ();
	}

	void ChannelsManager::LeaveChannel (const QString& channel, const QString& msg)
	{
		ISH_->LeaveChannel (channel, msg);
	}

	void ChannelsManager::CloseChannel (const QString& channel)
	{
		if (const auto handler = GetChannelHandler (channel))
			handler->CloseChannel ();
	}

	void ChannelsManager::CloseAllChannels ()
	{
		auto handlers = std::move (ChannelHandlers_);
		ChannelHandlers_.clear ();

		for (const auto& handler : handlers)
			handler->CloseChannel ();
	}

	void ChannelsManager::UnregisterChannel (ChannelHandler *ich)
	{
		const auto wasntEmpty = ChannelHandlers_.remove (ich->GetChannelOptions ().ChannelName_);

		const auto autoDisconnect = XmlSettingsManager::Instance ().property ("AutoDisconnectFromServer").toBool ();
		if (ChannelHandlers_.isEmpty () &&
				wasntEmpty &&
				autoDisconnect)
			ISH_->DisconnectFromServer ();
	}

	QHash<QString, QObject*> ChannelsManager::GetParticipantsByNick (const QString& nick) const
	{
		QHash<QString, QObject*> result;
		for (const auto& ich : ChannelHandlers_)
			if (ich->IsUserExists (nick))
				result [ich->GetChannelOptions ().ChannelName_] = ich->GetParticipantEntry (nick).get ();
		return result;
	}

	void ChannelsManager::AddParticipant (const QString& channel, const QString& nick,
			const QString& user, const QString& host)
	{
		if (const auto handler = GetChannelHandler (channel))
			handler->SetChannelUser (nick, user, host);
	}

	void ChannelsManager::LeaveParticipant (const QString& channel,
			const QString& nick, const QString& msg)
	{
		if (const auto handler = GetChannelHandler (channel))
			handler->LeaveParticipant (nick, msg);
	}

	void ChannelsManager::QuitParticipant (const QString& nick, const QString& msg)
	{
		for (const auto& ch : ChannelHandlers_)
			if (ch->IsUserExists (nick))
				ch->LeaveParticipant (nick, msg);
	}

	void ChannelsManager::KickParticipant (const QString& channel,
			const QString& target, const QString& reason, const QString& who)
	{
		if (const auto handler = GetChannelHandler (channel))
			handler->KickParticipant (target, reason, who);
	}

	void ChannelsManager::KickCommand (const QString& channel,
			const QString& nick, const QString& reason)
	{
		ISH_->KickParticipant (channel, nick, reason);
	}

	void ChannelsManager::ChangeNickname (const QString& oldNick, const QString& newNick)
	{
		for (const auto& ich : ChannelHandlers_)
			if (ich->IsUserExists (oldNick))
				ich->ChangeNickname (oldNick, newNick);
	}

	void ChannelsManager::GotNames (const QString& channel, const QStringList& participants)
	{
		if (const auto handler = GetChannelHandler (channel);
			handler && !handler->IsRosterReceived ())
		{
			for (const auto& nick : participants)
				if (!nick.isEmpty ())
					handler->SetChannelUser (nick);
		}
		else
			ReceiveCmdAnswerMessage (participants.join (' '));
	}

	void ChannelsManager::GotEndOfNamesCmd (const QString& channel)
	{
		if (const auto handler = GetChannelHandler (channel);
			!handler->IsRosterReceived ())
		{
			handler->SetRosterReceived (true);
			emit ISH_->GetAccount ()->gotCLItems ({ handler->GetCLEntry () });
		}
		else
			ReceiveCmdAnswerMessage (QStringLiteral ("End of /NAMES"));
	}

	void ChannelsManager::SendPublicMessage (const QString& channel, const QString& msg)
	{
		LastActiveChannel_ = channel.toLower ();
		const auto handler = GetChannelHandler (LastActiveChannel_);
		if (msg.startsWith ('/'))
		{
			if (handler)
			{
				const auto& cmd = ISH_->ParseMessageForCommand (msg, LastActiveChannel_);
				if (cmd == "say"_ql)
					handler->HandleIncomingMessage (ISH_->GetNickName (), msg.mid (4));
				else if (cmd == "me"_ql)
					handler->HandleIncomingMessage (ISH_->GetNickName (), msg);
				else
					handler->HandleServiceMessage (msg,
							IMessage::Type::EventMessage,
							IMessage::SubType::Other);
			}
		}
		else
		{
			ISH_->SendPublicMessage (msg, LastActiveChannel_);
			if (handler)
				handler->HandleIncomingMessage (ISH_->GetNickName (), msg);
		}
	}

	void ChannelsManager::ReceivePublicMessage (const QString& channel,
			const QString& nick, const QString& msg)
	{
		if (const auto handler = GetChannelHandler (channel))
			handler->HandleIncomingMessage (nick, msg);
	}

	bool ChannelsManager::ReceiveCmdAnswerMessage (const QString& answer)
	{
		const auto handler = GetChannelHandler (LastActiveChannel_);
		if (!handler)
			return false;

		handler->HandleServiceMessage (answer,
				IMessage::Type::EventMessage,
				IMessage::SubType::Other);

		return true;
	}

	void ChannelsManager::SetMUCSubject (const QString& channel, const QString& topic)
	{
		if (const auto handler = GetChannelHandler (channel))
			handler->SetMUCSubject (topic);

		ReceiveCmdAnswerMessage (topic);
	}

	void ChannelsManager::SetTopic (const QString& channel, const QString& topic)
	{
		ISH_->SetTopic (channel, topic);
	}

	void ChannelsManager::CTCPReply (const QString& msg)
	{
		for (const auto& ich : ChannelHandlers_)
			ich->HandleServiceMessage (msg,
					IMessage::Type::ServiceMessage,
					IMessage::SubType::Other);
	}

	void ChannelsManager::CTCPRequestResult (const QString& msg)
	{
		for (const auto& ich : ChannelHandlers_)
			ich->HandleServiceMessage (msg,
					IMessage::Type::ServiceMessage,
					IMessage::SubType::Other);
	}

	void ChannelsManager::SetBanListItem (const QString& channel,
			const QString& mask, const QString& nick, const QDateTime& time)
	{
		if (const auto handler = GetChannelHandler (channel))
			handler->SetBanListItem (mask, nick, time);
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
		if (const auto handler = GetChannelHandler (channel))
			handler->SetInviteListItem (mask, nick, time);
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
		const auto handler = GetChannelHandler (channel);
		if (!handler)
		{
			qWarning () << Q_FUNC_INFO
					<< "no handler for channel"
					<< channel;
			return;
		}

		bool action = mode [0] == '+';

		const auto handleRole = [&] (ChannelRole role)
		{
			if (const auto& entry = handler->GetExistingParticipantEntry (value))
			{
				if (action)
					entry->SetRole (role);
				else
					entry->RemoveRole (role);
				handler->MakePermsChangedMessage (value, role, action);
			}
		};

		for (int i = 1; i < mode.length (); ++i)
		{
			switch (mode [i].toLatin1 ())
			{
			case 'o':
				handleRole (ChannelRole::Operator);
				break;
			case 'v':
				handleRole (ChannelRole::Voiced);
				break;
			case 'a':
				// may be it is nessesary
				break;
			case 'i':
				handler->SetInviteMode (action);
				break;
			case 'm':
				handler->SetModerateMode (action);
				break;
			case 'n':
				handler->SetBlockOutsideMessagesMode (action);
				break;
			case 'q':
				if (ISH_->GetISupport ().value ("PREFIX").toByteArray ().contains ('q'))
					handleRole (ChannelRole::Owner);
				break;
			case 'p':
				handler->SetPrivateMode (action);
				break;
			case 'r':
				handler->SetServerReOpMode (action);
				break;
			case 's':
				handler->SetSecretMode (action);
				break;
			case 't':
				handler->SetOnlyOpTopicChangeMode (action);
				break;
			case 'l':
				handler->SetUserLimit (action, value.toInt ());
				break;
			case 'k':
				handler->SetChannelKey (action, value);
				break;
			case 'b':
				ISH_->ShowAnswer ("Mode", tr ("%1 added to your ban list.").arg (value));
				break;
			case 'e':
				ISH_->ShowAnswer ("Mode", tr ("%1 added to your except list.").arg (value));
				break;
			case 'I':
				ISH_->ShowAnswer ("Mode", tr ("%1 added to your invite list.").arg (value));
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

	QHash<QByteArray, QVariant> ChannelsManager::GetISupport () const
	{
		return ISH_->GetISupport ();
	}

	void ChannelsManager::SetPrivateChat (const QString& nick) const
	{
		for (const auto entryObj : GetParticipantsByNick (nick))
			if (const auto entry = qobject_cast<IrcParticipantEntry*> (entryObj))
				entry->SetPrivateChat (true);
	}

	void ChannelsManager::CreateServerParticipantEntry (const QString& nick)
	{
		ISH_->CreateServerParticipantEntry (nick);
	}

	void ChannelsManager::UpdateEntry (const WhoMessage& message)
	{
		if (const auto handler = GetChannelHandler (message.Channel_))
			handler->UpdateEntry (message);
	}

	int ChannelsManager::GetChannelUsersCount (const QString& channel)
	{
		const auto handler = GetChannelHandler (channel);
		return handler ? handler->GetParticipants ().size () : 0;
	}

	void ChannelsManager::ClosePrivateChat (const QString& nick)
	{
		ISH_->ClosePrivateChat (nick);
	}

	void ChannelsManager::SetChannelUrl (const QString& channel, const QString& url)
	{
		if (const auto handler = GetChannelHandler (channel))
			handler->SetUrl (url);
	}

	void ChannelsManager::SetTopicWhoTime (const QString& channel, const QString& who, quint64 time)
	{
		const auto handler = GetChannelHandler (channel);
		if (!handler)
			return;

		const auto& msg = tr ("Topic was set by %1 at %2")
				.arg (who,
					  QDateTime::fromSecsSinceEpoch (time).toString (Qt::TextDate));
		handler->HandleServiceMessage (msg,
				IMessage::Type::ServiceMessage,
				IMessage::SubType::Other);
	}
}
