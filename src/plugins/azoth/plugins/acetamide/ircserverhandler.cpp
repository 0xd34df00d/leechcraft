/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ircserverhandler.h"
#include <QTimer>
#include <util/util.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/xpc/notificationactionhandler.h>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include "channelhandler.h"
#include "channelclentry.h"
#include "servercommandmessage.h"
#include "clientconnection.h"
#include "ircaccount.h"
#include "ircmessage.h"
#include "ircparser.h"
#include "ircserverclentry.h"
#include "xmlsettingsmanager.h"
#include "channelpublicmessage.h"
#include "ircerrorhandler.h"
#include "ircserversocket.h"
#include "rplisupportparser.h"
#include "channelsmanager.h"
#include "channelslistdialog.h"
#include "nickservidentifymanager.h"
#include "parsers.h"

namespace LC::Azoth::Acetamide
{
	IrcServerHandler::IrcServerHandler (const ServerOptions& server, IrcAccount *account)
	: Account_ { account }
	, ErrorHandler_ { new IrcErrorHandler { this } }
	, IrcParser_ { new IrcParser { this } }
	, ServerCLEntry_ { new IrcServerCLEntry { this, account } }
	, CmdManager_ { this, IrcParser_ }
	, ServerResponseManager_ { this }
	, ChannelsManager_ { new ChannelsManager { this } }
	, ServerID_ { server.ServerName_ + ":" + QString::number (server.ServerPort_) }
	, NickName_ { server.ServerNickName_ }
	, ServerOptions_ { server }
	, AutoWhoTimer_ { new QTimer { this } }
	{
		XmlSettingsManager::Instance ().RegisterObject ("AutoWhoPeriod",
				this, "handleUpdateWhoPeriod");
		XmlSettingsManager::Instance ().RegisterObject ("AutoWhoRequest",
				this, "handleSetAutoWho");

		connect (AutoWhoTimer_,
				&QTimer::timeout,
				this,
				&IrcServerHandler::autoWhoRequest);

		handleSetAutoWho ();
	}

	IrcServerHandler::~IrcServerHandler () = default;

	IrcServerCLEntry* IrcServerHandler::GetCLEntry () const
	{
		return ServerCLEntry_;
	}

	IrcAccount* IrcServerHandler::GetAccount () const
	{
		return Account_;
	}

	IrcParser* IrcServerHandler::GetParser () const
	{
		return IrcParser_;
	}

	ChannelsManager* IrcServerHandler::GetChannelManager () const
	{
		return ChannelsManager_;
	}

	QString IrcServerHandler::GetNickName () const
	{
		return NickName_;
	}

	QString IrcServerHandler::GetServerID () const
	{
		return ServerID_;
	}

	ServerOptions IrcServerHandler::GetServerOptions () const
	{
		return ServerOptions_;
	}

	QObjectList IrcServerHandler::GetCLEntries () const
	{
		QObjectList result;

		if (ChannelsManager_)
			result << ChannelsManager_->GetCLEntries ();

		for (const auto& spe : Nick2Entry_)
			result << spe.get ();

		return result;
	}

	QList<ChannelHandler*> IrcServerHandler::GetChannelHandlers () const
	{
		return ChannelsManager_->GetChannels ();
	}

	IrcMessage* IrcServerHandler::CreateMessage (IMessage::Type type,
			const QString& variant, const QString& body)
	{
		const auto msg = new IrcMessage (type,
				IMessage::Direction::In,
				variant,
				QString (),
				Account_->GetClientConnection ().get ());
		msg->SetBody (body);
		msg->SetDateTime (QDateTime::currentDateTime ());

		return msg;
	}

	bool IrcServerHandler::IsChannelExists (const QString& channel) const
	{
		return ChannelsManager_->IsChannelExists (channel);
	}

	void IrcServerHandler::SetNickName (const QString& nick)
	{
		NickName_ = nick;
	}

	void IrcServerHandler::Add2ChannelsQueue (const ChannelOptions& ch)
	{
		if (!ch.ChannelName_.isEmpty ())
			ChannelsManager_->AddChannel2Queue (ch);
	}

	void IrcServerHandler::JoinChannel (const ChannelOptions& channel)
	{
		if (ServerConnectionState_ == Connected)
		{
			if (!IsChannelExists (channel.ChannelName_))
				IrcParser_->JoinCommand ({ channel.ChannelName_, channel.ChannelPassword_ });
		}
		else
			Add2ChannelsQueue (channel);
	}

	bool IrcServerHandler::JoinedChannel (const ChannelOptions& channel)
	{
		const auto& channelName = channel.ChannelName_.toLower ();
		if (ServerConnectionState_ == Connected &&
				!IsChannelExists (channelName))
			return ChannelsManager_->AddChannel (channel);

		Add2ChannelsQueue (channel);
		return false;
	}

	void IrcServerHandler::JoinParticipant (const QString& nick,
			const QString& msg, const QString& user, const QString& host)
	{
		if (Nick2Entry_.contains (nick))
			ClosePrivateChat (nick);
		ChannelsManager_->AddParticipant (msg.toLower (), nick, user, host);

		IrcParser_->WhoCommand (QStringList (nick));
		SpyWho_ [nick] = AnswersOnWhoCommand;
	}

	void IrcServerHandler::CloseChannel (const QString& channel)
	{
		ChannelsManager_->CloseChannel (channel.toLower ());
	}

	void IrcServerHandler::LeaveParticipant (const QString& nick,
			const QString& channel, const QString& msg)
	{
		ChannelsManager_->LeaveParticipant (channel, nick, msg);
	}

	void IrcServerHandler::SendQuit ()
	{
		if (ServerConnectionState_ == Connected)
			IrcParser_->QuitCommand ({ Account_->GetClientConnection ()->GetStatusStringForState (SOffline) });

		if (Socket_)
			Socket_->Close ();
	}

	void IrcServerHandler::QuitParticipant (const QString& nick, const QString& msg)
	{
		ChannelsManager_->QuitParticipant (nick, msg);
		if (Nick2Entry_.contains (nick))
			Nick2Entry_.remove (nick);
	}

	void IrcServerHandler::SendMessage (const QStringList& cmd)
	{
		if (cmd.isEmpty ())
			return;

		const auto& target = cmd.first ();
		const auto& msg = cmd.mid (1);

		if (IsChannelExists (target))
			ChannelsManager_->SendPublicMessage (target.toLower (), msg.join (' '));
		else
			IrcParser_->PrivMsgCommand (cmd);
	}

	void IrcServerHandler::IncomingMessage (const QString& nick,
			const QString& target, const QString& msg, IMessage::Type type)
	{
		if (ChannelsManager_->IsChannelExists (target))
			ChannelsManager_->ReceivePublicMessage (target, nick, msg);
		else
		{
			//TODO Work only for exists entries
			const auto message = new IrcMessage (type,
					IMessage::Direction::In,
					ServerID_,
					nick,
					Account_->GetClientConnection ().get ());
			message->SetBody (msg);
			message->SetDateTime (QDateTime::currentDateTime ());

			bool found = false;
			for (const auto entryObj : ChannelsManager_->GetParticipantsByNick (nick))
			{
				const auto entry = qobject_cast<EntryBase*> (entryObj);
				if (!entry)
					continue;

				found = true;
				entry->HandleMessage (message);
			}

			if (!found)
			{
				if (Nick2Entry_.contains (nick))
					Nick2Entry_ [nick]->HandleMessage (message);
				else
					GetParticipantEntry (nick)->HandleMessage (message);
			}
		}
	}

	void IrcServerHandler::IncomingNoticeMessage (const QString& nick, const QString& msg)
	{
		ShowAnswer ("NOTICE", msg);
		const auto& nsim = Account_->GetParentProtocol ()->GetNickServIdentifyManager ();
		const auto& list = nsim.GetIdentifies (ServerOptions_.ServerName_,
				GetNickName (),
				nick);

		if (list.isEmpty ())
			return;

		for (const auto& nsi : list)
		{
			const auto authRegExp = QRegularExpression::fromWildcard (nsi.AuthString_, Qt::CaseInsensitive);
			if (msg.contains (authRegExp))
			{
				SendMessage2Server (nsi.AuthMessage_);
				break;
			}
		}
	}

	void IrcServerHandler::ChangeNickname (const QString& nick,
			const QString& msg)
	{
		ChannelsManager_->ChangeNickname (nick, msg);

		if (nick == NickName_)
			NickName_ = msg;

		if (const auto entry = Nick2Entry_.take (nick))
		{
			Nick2Entry_.insert (msg, entry);
			entry->SetEntryName (msg);
		}
	}

	void IrcServerHandler::GetBanList (const QString& channel)
	{
		IrcParser_->ChanModeCommand ({ channel, QString { 'b' } });
	}

	void IrcServerHandler::GetExceptList (const QString& channel)
	{
		IrcParser_->ChanModeCommand ({ channel, QString { 'e' } });
	}

	void IrcServerHandler::GetInviteList (const QString& channel)
	{
		IrcParser_->ChanModeCommand ({ channel, QString { 'I' } });
	}

	void IrcServerHandler::AddBanListItem (const QString& channel, const QString& mask)
	{
		IrcParser_->ChanModeCommand ({ channel, QStringLiteral ("+b"), mask });
	}

	void IrcServerHandler::RemoveBanListItem (const QString& channel, const QString& mask)
	{
		IrcParser_->ChanModeCommand ({ channel, QStringLiteral ("-b"), mask });
	}

	void IrcServerHandler::AddExceptListItem (const QString& channel, const QString& mask)
	{
		IrcParser_->ChanModeCommand ({ channel, QStringLiteral ("+e"), mask });
	}

	void IrcServerHandler::RemoveExceptListItem (const QString& channel, const QString& mask)
	{
		IrcParser_->ChanModeCommand ({ channel, QStringLiteral ("-e"), mask });
	}

	void IrcServerHandler::AddInviteListItem (const QString& channel, const QString& mask)
	{
		IrcParser_->ChanModeCommand ({ channel, QStringLiteral ("+I"), mask });
	}

	void IrcServerHandler::RemoveInviteListItem (const QString& channel, const QString& mask)
	{
		IrcParser_->ChanModeCommand ({ channel, QStringLiteral ("-I"), mask });
	}

	void IrcServerHandler::SetNewChannelModes (const QString& channel, const ChannelModes& modes)
	{
		if (!IsChannelExists (channel))
			return;

		auto chanMode = [&] (bool enabled, char code) { return QStringList { channel, (enabled ? '+' : '-') + QString { code } }; };

		IrcParser_->ChanModeCommand (chanMode (modes.BlockOutsideMessageMode_, 'n'));

		if (modes.ChannelKey_.first)
			IrcParser_->ChanModeCommand ({ channel, "+k", modes.ChannelKey_.second });
		else
			IrcParser_->ChanModeCommand ({ channel, "-k" });

		IrcParser_->ChanModeCommand (chanMode (modes.InviteMode_, 'i'));
		IrcParser_->ChanModeCommand (chanMode (modes.ModerateMode_, 'm'));
		IrcParser_->ChanModeCommand (chanMode (modes.OnlyOpChangeTopicMode_, 't'));
		IrcParser_->ChanModeCommand (chanMode (modes.PrivateMode_, 'p'));
		IrcParser_->ChanModeCommand (chanMode (modes.ReOpMode_, 'r'));
		IrcParser_->ChanModeCommand (chanMode (modes.SecretMode_, 's'));

		if (modes.UserLimit_.first)
			IrcParser_->ChanModeCommand ({ channel, QStringLiteral ("+l"), QString::number (modes.UserLimit_.second) });
		else
			IrcParser_->ChanModeCommand ({ channel, QStringLiteral ("-l") });
	}

	void IrcServerHandler::SetNewChannelMode (const QString& channel,
			const QString& mode, const QString& target)
	{
		if (!IsChannelExists (channel))
			return;

		IrcParser_->ChanModeCommand ({ channel, mode, target });
	}

	void IrcServerHandler::PongMessage (const QString& msg)
	{
		IrcParser_->PongCommand ({ msg });
	}

	void IrcServerHandler::SetTopic (const QString& channel, const QString& topic)
	{
		IrcParser_->TopicCommand ({ channel, topic });
	}

	void IrcServerHandler::GotTopic (const QString& channel,
			const QString& message)
	{
		if (IsChannelExists (channel))
			ChannelsManager_->SetMUCSubject (channel, message);
		else
			ShowAnswer ("TOPIC", message);
	}

	void IrcServerHandler::GotKickCommand (const QString& nick,
			const QString& channel, const QString& target,
			const QString& msg)
	{
		if (IsChannelExists (channel))
			ChannelsManager_->KickParticipant (channel, target, msg, nick);
	}

	void IrcServerHandler::KickParticipant (const QString& channel,
			const QString& nick, const QString& reason)
	{
		if (IsChannelExists (channel))
			IrcParser_->KickCommand ({ channel, nick, reason });
	}

	void IrcServerHandler::GotInvitation (const QString& nick,
			const QString& msg)
	{
		if (InviteChannelsDialog_)
		{
			InviteChannelsDialog_->AddInvitation (msg, nick);
			return;
		}

		InviteChannelsDialog_ = std::make_unique<InviteChannelsDialog> (msg, nick);
		connect (InviteChannelsDialog_.get (),
				&QDialog::accepted,
				this,
				&IrcServerHandler::joinAfterInvite);
		connect (InviteChannelsDialog_.get (),
				&QDialog::finished,
				this,
				[this] { InviteChannelsDialog_.reset (); },
				Qt::QueuedConnection);
		InviteChannelsDialog_->show ();
	}

	void IrcServerHandler::ShowAnswer (const QByteArray& cmd,
			const QString& answer, bool /*isEndOf*/, IMessage::Type type)
	{
		auto msg = "[" + cmd.toUpper () + "] " + answer;
		bool res = ChannelsManager_->ReceiveCmdAnswerMessage (msg);

		if (!res ||
				XmlSettingsManager::Instance ()
						.property ("ServerDuplicateCommandAnswer").toBool ())
			ServerCLEntry_->HandleMessage (CreateMessage (type,
					ServerID_,
					msg));
	}

	void IrcServerHandler::CTCPReply (const QString& nick,
			const QString& cmd, const QString& mess)
	{
		ChannelsManager_->CTCPReply (mess);
		IrcParser_->CTCPReply (QStringList () << nick << cmd);
	}

	void IrcServerHandler::CTCPRequestResult (const QString& msg)
	{
		ChannelsManager_->CTCPRequestResult (msg);
	}

	void IrcServerHandler::CTCPRequst (const QStringList& cmd)
	{
		IrcParser_->CTCPRequest (cmd);
	}

	void IrcServerHandler::GotNames (const QString& channel,
			const QStringList& participants)
	{
		ChannelsManager_->GotNames (channel.toLower (), participants);
	}

	void IrcServerHandler::GotEndOfNames (const QString& channel)
	{
		ChannelsManager_->GotEndOfNamesCmd (channel.toLower ());
	}

	void IrcServerHandler::ShowUserHost (const QString& nick,
			const QString& host)
	{
		ShowAnswer ("userhost", tr ("%1 is a %2").arg (nick, host));
	}

	void IrcServerHandler::ShowIsUserOnServer (const QString& nick)
	{
		ShowAnswer ("ison", tr ("%1 is on server").arg (nick));
	}

	void IrcServerHandler::HandleSpyNick (const WhoIsMessage& msg)
	{
		auto& whois = SpyNick2WhoIsMessage_ [msg.Nick_];
		auto merge = [&whois, &msg] (auto mem)
		{
			if (!(msg.*mem).isEmpty ())
				whois.*mem = msg.*mem;
		};
		merge (&WhoIsMessage::UserName_);
		merge (&WhoIsMessage::Host_);
		merge (&WhoIsMessage::RealName_);
		merge (&WhoIsMessage::Channels_);
		merge (&WhoIsMessage::ServerName_);
		merge (&WhoIsMessage::ServerCountry_);

		if (!msg.EndString_.isEmpty ())
		{
			for (const auto entryObj : ChannelsManager_->GetParticipantsByNick (msg.Nick_))
				if (const auto entry = dynamic_cast<ChannelParticipantEntry*> (entryObj))
					entry->SetInfo (whois);

			SpyNick2WhoIsMessage_.remove (msg.Nick_);
		}
	}

	void IrcServerHandler::ShowWhoIsReply (const WhoIsMessage& msg, bool isEndOf)
	{
		if (msg.Nick_.isEmpty ())
			return;

		if (SpyNick2WhoIsMessage_.contains (msg.Nick_))
		{
			HandleSpyNick (msg);
			return;
		}

		auto show = [&] (const QString& msg) { ShowAnswer ("whois", msg, isEndOf); };

		if (!msg.UserName_.isEmpty () &&
				!msg.Host_.isEmpty ())
			show (tr ("%1 is %2").arg (msg.Nick_, msg.Nick_ + "!" + msg.UserName_ + "@" + msg.Host_));

		if (!msg.RealName_.isEmpty ())
			show (tr ("%1's real name is %2").arg (msg.Nick_, msg.RealName_));

		if (!msg.Channels_.isEmpty ())
			show (tr ("%1 is on channels: %2").arg (msg.Nick_, msg.Channels_.join (u", "_qsv)));

		if (!msg.ServerName_.isEmpty () &&
				!msg.ServerCountry_.isEmpty ())
			show (tr ("%1's server is: %2 - %3").arg (msg.Nick_, msg.ServerName_, msg.ServerCountry_));

		if (!msg.IdleTime_.isEmpty ())
			show (tr ("%1's idle time: %2").arg (msg.Nick_, msg.IdleTime_));

		if (!msg.AuthTime_.isEmpty ())
			show (tr ("%1's auth date: %2").arg (msg.Nick_, msg.AuthTime_));

		if (!msg.IrcOperator_.isEmpty ())
			show (msg.Nick_ + ": " + msg.IrcOperator_);

		if (!msg.LoggedName_.isEmpty ())
			show (tr ("%1 is logged in as %2").arg (msg.Nick_, msg.LoggedName_));

		if (!msg.Secure_.isEmpty ())
			show (tr ("%1 is using a secure connection").arg (msg.Nick_));

		if (!msg.ConnectedFrom_.isEmpty ())
			show (tr ("%1: %2").arg (msg.Nick_, msg.ConnectedFrom_));

		if (!msg.IsHelpOp_.isEmpty ())
			show (tr ("%1 is available for help").arg (msg.Nick_));

		if (!msg.IsRegistered_.isEmpty ())
			show (tr ("%1 is a registered nick").arg (msg.Nick_));

		if (!msg.Mail_.isEmpty ())
			show (tr ("%1 e-mail address is %2").arg (msg.Nick_, msg.Mail_));

		if (!msg.EndString_.isEmpty ())
			show (msg.Nick_ + " " + msg.EndString_);
	}

	void IrcServerHandler::ShowWhoWasReply (const QString& msg, bool isEndOf)
	{
		ShowAnswer ("whowas", msg, isEndOf);
	}

	void IrcServerHandler::ShowWhoReply (const WhoMessage&  msg, bool isEndOf)
	{
		QString message;
		if (!msg.Nick_.isEmpty () &&
				!msg.EndString_.isEmpty ())
			message = msg.Nick_ + " " + msg.EndString_;
		else
			message = tr ("%1 [%2@%3]: Channel: %4, Server: %5, "
					"Hops: %6, Flags: %7, Away: %8, Real Name: %9")
							.arg (msg.Nick_,
									msg.UserName_,
									msg.Host_,
									msg.Channel_,
									msg.ServerName_,
									QString::number (msg.Jumps_),
									msg.Flags_,
									msg.IsAway_ ? "true" : "false",
									msg.RealName_);

		bool contains = false;
		QString key;
		if (SpyWho_.contains (msg.Channel_.toLower ()))
		{
			contains = true;
			key = msg.Channel_.toLower ();
		}
		else if (SpyWho_.contains (msg.Nick_))
		{
			contains = true;
			key = msg.Nick_;
		}
		else
			ShowAnswer ("who", message, isEndOf);

		if (contains)
		{
			if (!isEndOf)
				ChannelsManager_->UpdateEntry (msg);
			--SpyWho_ [key];
			if (!SpyWho_ [key])
				SpyWho_.remove (key);
		}
	}

	void IrcServerHandler::ShowLinksReply (const QString& msg, bool isEndOf)
	{
        ShowAnswer ("links", msg, isEndOf);
	}

	void IrcServerHandler::ShowInfoReply (const QString& msg, bool isEndOf)
	{
        ShowAnswer ("info", msg, isEndOf);
	}

	void IrcServerHandler::ShowMotdReply (const QString& msg, bool isEndOf)
	{
        ShowAnswer ("motd", msg, isEndOf);
	}

	void IrcServerHandler::ShowUsersReply (const QString& msg, bool isEndOf)
	{
        ShowAnswer ("users", msg, isEndOf);
	}

	void IrcServerHandler::ShowTraceReply (const QString& msg, bool isEndOf)
	{
        ShowAnswer ("trace", msg, isEndOf);
	}

	void IrcServerHandler::ShowStatsReply (const QString& msg, bool isEndOf)
	{
        ShowAnswer ("stats", msg, isEndOf);
	}

	void IrcServerHandler::ShowBanList (const QString& channel,
			const QString& mask, const QString& nick, const QDateTime& time)
	{
		if (IsChannelExists (channel))
			ChannelsManager_->SetBanListItem (channel, mask, nick, time);
	}

	void IrcServerHandler::ShowBanListEnd (const QString& msg)
	{
		ShowAnswer ("mode", msg);
	}

	void IrcServerHandler::ShowExceptList (const QString& channel,
			const QString& mask, const QString& nick, const QDateTime& time)
	{
		if (!IsChannelExists (channel))
			return;

		ChannelsManager_->SetExceptListItem (channel, mask, nick, time);
	}

	void IrcServerHandler::ShowExceptListEnd (const QString& msg)
	{
		ShowAnswer ("MODE", msg);
	}

	void IrcServerHandler::ShowInviteList (const QString& channel,
			const QString& mask, const QString& nick, const QDateTime& time)
	{
		if (IsChannelExists (channel))
			ChannelsManager_->SetInviteListItem (channel, mask, nick, time);
	}

	void IrcServerHandler::ShowInviteListEnd (const QString& msg)
	{
		ShowAnswer ("mode", msg);
	}

	void IrcServerHandler::SendPublicMessage (const QString& msg,
			const QString& channel)
	{
		for (const auto& str : msg.split ('\n'))
			IrcParser_->PrivMsgCommand ({ channel, str });
	}

	void IrcServerHandler::SendPrivateMessage (IrcMessage* msg)
	{
		for (const auto& str : msg->GetBody ().split ('\n'))
			IrcParser_->PrivMsgCommand ({ msg->GetOtherVariant (), str });

		bool found = false;
		for (const auto entryObj : ChannelsManager_->
				GetParticipantsByNick (msg->GetOtherVariant ()))
		{
			const auto entry = qobject_cast<EntryBase*> (entryObj);
			if (!entry)
				continue;

			found = true;
			entry->HandleMessage (msg);
		}

		if (!found &&
				Nick2Entry_.contains (msg->GetOtherVariant ()))
			Nick2Entry_ [msg->GetOtherVariant ()]->HandleMessage (msg);
	}

	void IrcServerHandler::SendMessage2Server (const QString& msg)
	{
		const auto& unslashed = msg.startsWith ('/') ? msg.mid (1) : msg;
		const auto& cmd = CmdManager_.VerifyMessage (msg, {});
		if (cmd.isEmpty ())
			SendCommand (unslashed);
		ShowAnswer (cmd, msg);
	}

	QString IrcServerHandler::ParseMessageForCommand (const QString& msg, const QString& channel) const
	{
		const auto& cmd = CmdManager_.VerifyMessage (msg, channel);
		if (cmd.isEmpty ())
			SendCommand (msg.startsWith ('/') ? msg.mid (1) : msg);
		return cmd;
	}

	void IrcServerHandler::LeaveChannel (const QString& channel,
			const QString& msg)
	{
		IrcParser_->PartCommand (QStringList () << channel << msg);
	}

	void IrcServerHandler::ConnectToServer ()
	{
		if (ServerConnectionState_ != NotConnected)
			return;

		Socket_ = std::make_unique<IrcServerSocket> (this);
		Socket_->ConnectToHost (ServerOptions_.ServerName_, ServerOptions_.ServerPort_);
		ServerConnectionState_ = InProgress;

		connect (Socket_.get (),
				&IrcServerSocket::connected,
				this,
				[this]
				{
					ServerConnectionState_ = Connected;
					Account_->GetClientConnection ()->serverConnected (ServerID_);
					ServerCLEntry_->SetStatus (EntryStatus (SOnline, QString ()));
					IrcParser_->AuthCommand ();

					for (const auto& handler : GetChannelHandlers ())
						JoinChannel (handler->GetChannelOptions ());
				});
		connect (Socket_.get (),
				&IrcServerSocket::disconnected,
				this,
				[this, socket = Socket_.get ()]
				{
					ServerConnectionState_ = NotConnected;
					ServerCLEntry_->SetStatus (EntryStatus (SOffline, QString ()));
					socket->Close ();
					Account_->GetClientConnection ()->serverDisconnected (ServerID_);
				});

		connect (Socket_.get (),
				&IrcServerSocket::retriableSocketError,
				this,
				[this] (QAbstractSocket::SocketError, const QString& errorString)
				{
					ServerConnectionState_ = NotConnected;
					ServerCLEntry_->SetStatus (EntryStatus (SError, errorString));

					const auto& e = Util::MakeNotification (Lits::AzothAcetamide,
							errorString,
							Priority::Warning);
					GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
				});
		connect (Socket_.get (),
				&IrcServerSocket::finalSocketError,
				this,
				[this] (QAbstractSocket::SocketError, const QString& errorString)
				{
					ServerConnectionState_ = NotConnected;
					ServerCLEntry_->SetStatus (EntryStatus (SError, errorString));

					const auto& e = Util::MakeNotification (Lits::AzothAcetamide,
							errorString,
							Priority::Critical);
					GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
				});
		connect (Socket_.get (),
				&IrcServerSocket::sslErrors,
				Account_,
				&IrcAccount::sslErrors);
	}

	void IrcServerHandler::DisconnectFromServer ()
	{
		Account_->ChangeState (EntryStatus (SOffline, QString ()));
		ChannelsManager_->CloseAllChannels ();

		const auto& entries = Util::Map (Nick2Entry_, [] (const auto& entry) -> QObject* { return entry.get (); });
		emit Account_->removedCLItems (entries);

		Nick2Entry_.clear ();

		if (Socket_ && ServerConnectionState_ != NotConnected)
			Socket_->DisconnectFromHost ();
	}

	void IrcServerHandler::SendCommand (const QString& cmd) const
	{
		SendToConsole (IMessage::Direction::Out, cmd);
		if (Socket_)
			Socket_->Send (cmd + "\r\n");
	}

	void IrcServerHandler::SendToConsole (IMessage::Direction dir, const QString& message) const
	{
		if (!IsConsoleEnabled_)
			return;

		emit sendMessageToConsole (dir, message);
	}

	void IrcServerHandler::NickCmdError ()
	{
		if (!Account_)
			return;
		
		if (Account_->GetNickNames ().isEmpty ())
		{
			qDebug () << Q_FUNC_INFO << "NickName conflict";
			emit ServerCLEntry_->nicknameConflict (NickName_);
			return;
		}
		
		if (LastNickIndex_ < Account_->GetNickNames ().count ())
			NickName_ = Account_->GetNickNames ().at (LastNickIndex_++);
		else
		{
			qDebug () << Q_FUNC_INFO << "NickName conflict";
			emit ServerCLEntry_->nicknameConflict (NickName_);
			return;
		}
		
		if (NickName_.isEmpty ())
		{
			NickCmdError ();
			return;
		}

		IrcParser_->NickCommand ({ NickName_ });
	}

	ServerParticipantEntry_ptr IrcServerHandler::GetParticipantEntry (const QString& nick)
	{
		if (auto entry = Nick2Entry_.value (nick))
			return entry;

		auto entry = CreateParticipantEntry (nick);
		Nick2Entry_ [nick] = entry;
		return entry;
	}

	void IrcServerHandler::RemoveParticipantEntry (const QString& nick)
	{
		Nick2Entry_.remove (nick);
	}

	void IrcServerHandler::SetConsoleEnabled (bool enabled)
	{
		IsConsoleEnabled_ = enabled;
	}

	void IrcServerHandler::ReadReply (const QString& msg)
	{
		SendToConsole (IMessage::Direction::In, msg.trimmed ());
		const auto& maybeOpts = ParseMessage (msg);
		if (!maybeOpts)
		{
			qWarning () << "unable to parse IRC command"
					<< msg;
			return;
		}

		const auto& opts = *maybeOpts;
		if (ErrorHandler_->IsError (opts.Command_.toInt ()))
		{
			ErrorHandler_->HandleError (opts);
			if (opts.Command_ == "433"_ql)
				NickCmdError ();
		}
		else
			ServerResponseManager_.DoAction (opts);
	}

	ServerParticipantEntry_ptr IrcServerHandler::CreateParticipantEntry (const QString& nick)
	{
		auto entry = std::make_shared<ServerParticipantEntry> (nick, this, Account_);
		emit Account_->gotCLItems ({ entry.get () });
		entry->SetStatus (EntryStatus (SOnline, QString ()));
		return entry;
	}

	void IrcServerHandler::JoinFromQueue ()
	{
		for (const auto& co : ChannelsManager_->GetChannelsQueue ())
			IrcParser_->JoinCommand ({ co.ChannelName_, co.ChannelPassword_ });

		ChannelsManager_->CleanQueue ();
	}

	void IrcServerHandler::SayCommand (const QStringList& params)
	{
		if (params.isEmpty ())
			return;

		const QString& channel = params.first ();
		SendPublicMessage (params.mid (1).join (' '), channel.toLower ());
	}

	void IrcServerHandler::ParseChanMode (const QString& channel,
			const QString& mode, const QString& value)
	{
		if (mode.isEmpty ())
			return;

		if (IsChannelExists (channel))
			ChannelsManager_->ParseChanMode (channel, mode, value);
	}

	void IrcServerHandler::ParseUserMode (const QString& nick,
			const QString& mode)
	{
		Q_UNUSED (nick);
		Q_UNUSED (mode);
		//TODO but I don't know how it use
	}

	void IrcServerHandler::ParserISupport (const QString& msg)
	{
		if (auto res = ParseISupportReply (msg))
		{
			ISupport_.reserve (ISupport_.size () + res->size ());

			for (auto&& [key, value] : Util::Stlize (*res))
				ISupport_ [key] = std::move (value);
		}
	}

	QHash<QByteArray, QVariant> IrcServerHandler::GetISupport () const
	{
		return ISupport_;
	}

	void IrcServerHandler::RequestWhoIs (const QString& nick)
	{
		IrcParser_->WhoisCommand ({ nick });
	}

	void IrcServerHandler::RequestWhoWas (const QString& nick)
	{
		IrcParser_->WhowasCommand ({ nick });
	}

	void IrcServerHandler::RequestWho (const QString& nick)
	{
		IrcParser_->WhoCommand ({ nick });
	}

	void IrcServerHandler::ClosePrivateChat (const QString& nick)
	{
		if (const auto entry = Nick2Entry_.take (nick))
			emit Account_->removedCLItems ({ entry.get () });

		for (const auto entryObj : ChannelsManager_->GetParticipantsByNick (nick))
			if (const auto entry = dynamic_cast<IrcParticipantEntry*> (entryObj))
				entry->SetPrivateChat (false);
	}

	void IrcServerHandler::CreateServerParticipantEntry (const QString& nick)
	{
		GetParticipantEntry (nick)->SetStatus (EntryStatus (SOnline, {}));
	}

	void IrcServerHandler::VCardRequest (const QString& nick)
	{
		RequestWhoIs (nick);

		WhoIsMessage msg;
		msg.Nick_ = nick;

		SpyNick2WhoIsMessage_.insert (nick, msg);
	}

	void IrcServerHandler::SetAway (const QString& message)
	{
		IrcParser_->AwayCommand (QStringList (message));
	}

	void IrcServerHandler::ChangeAway (bool away, const QString& message)
	{
		away ?
			Account_->SetState (EntryStatus (SAway, message)) :
			Account_->SetState (EntryStatus (SOnline, QString ()));
		autoWhoRequest ();
	}

	void IrcServerHandler::GotChannelUrl (const QString& channel, const QString& url)
	{
		ChannelsManager_->SetChannelUrl (channel, url);
	}

	void IrcServerHandler::GotTopicWhoTime (const QString& channel,
			const QString& who, quint64 time)
	{
		ChannelsManager_->SetTopicWhoTime (channel, who, time);
	}

	void IrcServerHandler::SetIrcServerInfo (IrcServer server, const QString& version)
	{
		ServerOptions_.IrcServer_ = server;
		ServerOptions_.IrcServerVersion_ = version;
	}

	void IrcServerHandler::GotChannelsListBegin (const IrcMessageOptions&)
	{
		emit gotChannelsBegin ();
	}

	void IrcServerHandler::GotChannelsList (const IrcMessageOptions& opts)
	{
		ChannelsDiscoverInfo info;
		info.Topic_ = opts.Message_;
		info.ChannelName_ = opts.Parameters_.value (1);
		info.UsersCount_ = opts.Parameters_.value (2).toInt ();
		emit gotChannels (info);
	}

	void IrcServerHandler::GotChannelsListEnd (const IrcMessageOptions&)
	{
		emit gotChannelsEnd ();
	}

	void IrcServerHandler::joinAfterInvite ()
	{
		for (const auto& channel : InviteChannelsDialog_->GetChannels ())
		{
			ChannelOptions co;
			co.ChannelName_ = channel;
			co.ChannelPassword_ = QString ();
			co.ServerName_ = ServerOptions_.ServerName_;
			JoinChannel (co);
		}
	}

	void IrcServerHandler::autoWhoRequest ()
	{
		for (auto channel : ChannelsManager_->GetChannels ())
		{
			const auto& channelName = channel->GetChannelOptions().ChannelName_.toLower ();
			IrcParser_->WhoCommand ({ channelName });
			SpyWho_ [channelName] = ChannelsManager_->GetChannelUsersCount (channelName) + 1;
		}
	}

	void IrcServerHandler::showChannels (const QStringList&)
	{
		IrcParser_->ChannelsListCommand ({});

		const auto dlg = new ChannelsListDialog (this);
		dlg->setAttribute (Qt::WA_DeleteOnClose);
		dlg->show ();
	}

	using namespace std::chrono_literals;

	void IrcServerHandler::handleSetAutoWho ()
	{
		if (!XmlSettingsManager::Instance ().property ("AutoWhoRequest").toBool () &&
				AutoWhoTimer_->isActive ())
			AutoWhoTimer_->stop ();
		else if (XmlSettingsManager::Instance ().property ("AutoWhoRequest").toBool () &&
				!AutoWhoTimer_->isActive ())
			AutoWhoTimer_->start (XmlSettingsManager::Instance ().property ("AutoWhoPeriod").toInt () * 60s);
	}

	void IrcServerHandler::handleUpdateWhoPeriod ()
	{
		AutoWhoTimer_->setInterval (XmlSettingsManager::Instance ().property ("AutoWhoPeriod").toInt () * 60s);
	}
}
