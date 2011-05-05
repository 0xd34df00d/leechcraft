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

#include "ircserverhandler.h"
#include <boost/bind.hpp>
#include <QTextCodec>
#include <plugininterface/util.h>
#include <plugininterface/notificationactionhandler.h>
#include "channelhandler.h"
#include "channelclentry.h"
#include "channelpublicmessage.h"
#include "clientconnection.h"
#include "ircaccount.h"
#include "ircmessage.h"
#include "ircparser.h"
#include "ircserverclentry.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcServerHandler::IrcServerHandler (const ServerOptions& server,
			IrcAccount *account )
	: Account_ (account)
	, ServerOptions_ (server)
	, ServerCLEntry_ (new IrcServerCLEntry (this, account))
	, ServerID_ (server.ServerName_ + ":" +
			QString::number (server.ServerPort_))
	, ServerConnectionState_ (NotConnected)
	, NickName_ (server.ServerNickName_)
	, IsConsoleEnabled_ (false)
	, IsInviteDialogActive_ (false)
	, ChannelJoined_ (false)
	{
		IrcParser_ = new IrcParser (this);
		InitErrorsReplys ();
		InitCommandResponses ();

		connect (this,
				 SIGNAL (connected (const QString&)),
				 Account_->GetClientConnection ().get (),
				 SLOT (serverConnected (const QString&)));
	}

	IrcServerCLEntry* IrcServerHandler::GetCLEntry () const
	{
		return ServerCLEntry_;
	}

	IrcAccount* IrcServerHandler::GetAccount () const
	{
		return Account_;
	}

	QString IrcServerHandler::GetNickName () const
	{
		return NickName_;
	}

	IrcServerConsole_ptr IrcServerHandler::GetIrcServerConsole () const
	{
		return Console_;
	}

	QString IrcServerHandler::GetServerID_ () const
	{
		return ServerID_;
	}

	ServerOptions IrcServerHandler::GetServerOptions () const
	{
		return ServerOptions_;
	}

	ConnectionState IrcServerHandler::GetConnectionState () const
	{
		return ServerConnectionState_;
	}

	bool IrcServerHandler::IsChannelExists (const QString& channelID)
	{
		return ChannelHandlers_.contains (channelID);
	}

	bool IrcServerHandler::IsParticipantExists (const QString& nick)
	{
		return Nick2Entry_.contains (nick);
	}

	void IrcServerHandler::Add2ChannelsQueue (const ChannelOptions& ch)
	{
		if (!ChannelsQueue_.contains (ch))
			ChannelsQueue_ << ch;
	}

	void IrcServerHandler::SendPublicMessage (const QString& msg,
			const QString& channelId)
	{
		IrcParser_->PrivMsgCommand (EncodedMessage (msg, IMessage::DOut),
				ChannelHandlers_ [channelId]->GetChannelOptions ()
					.ChannelName_);
	}

	void IrcServerHandler::SendPrivateMessage (IrcMessage* msg)
	{
		IrcParser_->PrivMsgCommand
				(EncodedMessage (msg->GetBody (), IMessage::DOut),
				msg->GetOtherVariant ());
	}

	void IrcServerHandler::ParseMessageForCommand (const QString& msg,
			const QString& channelID)
	{
		QString commandMessage = EncodedMessage (msg.mid (1),
				IMessage::DOut);
		QString outputMessage = QString ();
		QStringList commandWithParams = commandMessage.split (' ');
		if (Name2Command_.contains (commandWithParams.at (0).toLower ()))
		{
			if (commandWithParams.at (0).toLower () == "invite")
				outputMessage = "You invite " + commandWithParams.at (1)
						+ " to a channel " + commandWithParams.at (2);
			else if (commandWithParams.at (0).toLower () == "quit")
				commandWithParams.append (ServerID_);
			else if (commandWithParams.at (0).toLower () == "me")
			{

				commandWithParams.insert (1, channelID
						.left (channelID.indexOf ('@')));
				commandWithParams.insert (2, "ACTION");
			}

			Name2Command_ [commandWithParams.at (0).toLower ()]
					(commandWithParams.mid (1));
		}

		if (!outputMessage.isEmpty ())
			Q_FOREACH (ChannelHandler *ich, ChannelHandlers_.values ())
				ich->ShowServiceMessage (outputMessage,
						IMessage::MTEventMessage,
						IMessage::MSTOther);
	}

	QList<QObject*> IrcServerHandler::GetCLEntries () const
	{
		QList<QObject*> result;
		Q_FOREACH (ChannelHandler *ich, ChannelHandlers_.values ())
			result << ich->GetCLEntry ();

		Q_FOREACH (ServerParticipantEntry_ptr spe, Nick2Entry_.values ())
			result << spe.get ();

		return result;
	}

	void IrcServerHandler::LeaveChannel (const QString& channels,
			const QString& msg)
	{
		IrcParser_->PartCommand (QStringList () << channels
				<< QString (":" + msg));
	}

	QStringList IrcServerHandler::GetPrivateChats () const
	{
		QStringList result;
		Q_FOREACH (ServerParticipantEntry_ptr spe, Nick2Entry_.values ())
			if (spe->IsPrivateChat ())
				result << spe->GetEntryName ();
		return result;
	}

	void IrcServerHandler::ClosePrivateChat (const QString& nick)
	{
		if (Nick2Entry_.contains (nick))
		{
			Account_->handleEntryRemoved (Nick2Entry_ [nick].get ());
			RemoveParticipantEntry (nick);
			if (!Nick2Entry_.count () && !ChannelHandlers_.count ())
				Account_->GetClientConnection ()->
						CloseServer (ServerID_);
		}
	}

	ChannelHandler*
			IrcServerHandler::GetChannelHandler (const QString& id)
	{
		return ChannelHandlers_.contains (id) ?
				ChannelHandlers_ [id] :
				0;
	}

	QList<ChannelHandler*> IrcServerHandler::GetChannelHandlers () const
	{
		return ChannelHandlers_.values ();
	}

	QList<ServerParticipantEntry_ptr>
			IrcServerHandler::GetParticipants (const QString& channel)
	{
		QList<ServerParticipantEntry_ptr> result;
		Q_FOREACH (ServerParticipantEntry_ptr spe, Nick2Entry_.values ())
			if (spe->GetChannels ().contains (channel))
				result << spe;
		return result;
	}

	bool IrcServerHandler::IsRoleAvailable (ChannelRole role)
	{
		return true;
	}

	IrcMessage*
			IrcServerHandler::CreateMessage (IMessage::MessageType type,
					const QString& variant, const QString& body)
	{
		IrcMessage *msg = new IrcMessage (type,
				IMessage::DIn,
				variant,
				QString (),
				Account_->GetClientConnection ().get ());

		msg->SetBody (body);
		msg->SetDateTime (QDateTime::currentDateTime ());

		return msg;
	}

	void IrcServerHandler::ConnectToServer ()
	{
		if (ServerConnectionState_ == NotConnected)
		{
			TcpSocket_ptr.reset (new QTcpSocket (this));
			InitSocket ();

			ServerConnectionState_ = InProgress;
			TcpSocket_ptr->connectToHost (ServerOptions_.ServerName_,
					ServerOptions_.ServerPort_);
		}
	}

	bool IrcServerHandler::DisconnectFromServer ()
	{
		if (ServerConnectionState_ != NotConnected)
		{
			TcpSocket_ptr->disconnectFromHost ();
			if (TcpSocket_ptr->state ()
					== QAbstractSocket::UnconnectedState &&
				TcpSocket_ptr->waitForDisconnected (1000))
			{
				ServerConnectionState_ = NotConnected;
				ServerCLEntry_->
						SetStatus (EntryStatus (SOffline, QString ()));
				TcpSocket_ptr->close ();
				return true;
			}
		}
		else
			return true;

		return false;
	}

	bool IrcServerHandler::JoinChannel (const ChannelOptions& channel)
	{
		QString id = QString (channel.ChannelName_ + "@" +
				channel.ServerName_).toLower ();
		if (ServerConnectionState_ == Connected)
		{
			ChannelHandler *ch = new ChannelHandler (this, channel);
			ChannelHandlers_ [id] = ch;
			IrcParser_->JoinCommand (channel.ChannelName_);

			ChannelCLEntry *ichEntry = ch->GetCLEntry ();

			if (!ichEntry)
				return false;

			Account_->handleGotRosterItems (QList<QObject*> () <<
					ichEntry);
		}
		else
			Add2ChannelsQueue (channel);

		return true;
	}

	void IrcServerHandler::JoinChannelByCmd (const QStringList& cmd)
	{
		Q_FOREACH (const QString& channel, cmd.at (0).split (','))
		{
			ChannelOptions co;
			co.ChannelName_ = channel;
			co.ChannelPassword_ = QString ();
			co.ServerName_ = ServerOptions_.ServerName_;

			JoinChannel (co);
		}
	}

	void IrcServerHandler::SendCommand (const QString& cmd)
	{
		qDebug () << TcpSocket_ptr.get () << cmd;
		SendToConsole (cmd.trimmed ());

		if (!TcpSocket_ptr->isWritable ())
		{
			qWarning () << Q_FUNC_INFO
					<< TcpSocket_ptr->error ()
					<< TcpSocket_ptr->errorString ();
			return;
		}

		if (TcpSocket_ptr->write (cmd.toAscii ()) == -1)
		{
			qWarning () << Q_FUNC_INFO
					<< TcpSocket_ptr->error ()
					<< TcpSocket_ptr->errorString ();
			return;
		}
	}

	void IrcServerHandler::IncomingMessage2Server ()
	{
		QString message;
		Q_FOREACH (std::string str, IrcParser_->GetIrcMessageOptions ()
				.Parameters_)
		{
			message.append (QString::fromUtf8 (str.c_str ()))
				.append (' ');
		}
		message.append (IrcParser_->GetIrcMessageOptions ().Message_);
		IrcMessage *msg = CreateMessage (IMessage::MTEventMessage,
				ServerID_, EncodedMessage (message, IMessage::DIn));

		ServerCLEntry_->HandleMessage (msg);
	}

	void IrcServerHandler::IncomingMessage2Channel ()
	{
		IrcMessageOptions imo = IrcParser_->GetIrcMessageOptions ();
		QString cmd = imo.Command_.toLower ();
		if (cmd == "privmsg" && IsCTCPMessage (imo.Message_))
			Command2Action_ ["ctcp_rpl"] (imo.Nick_, imo.Parameters_,
					imo.Message_);
		else if (cmd == "notice" && IsCTCPMessage (imo.Message_))
			Command2Action_ ["ctcp_rqst"] (imo.Nick_, imo.Parameters_,
					imo.Message_);
		else if (Command2Action_.contains (cmd))
			Command2Action_ [cmd] (imo.Nick_, imo.Parameters_,
					imo.Message_);

	}

	void IrcServerHandler::SendToConsole (const QString& message)
	{
		if (!IsConsoleEnabled_)
			return;

		IrcMessage *msg = CreateMessage (IMessage::MTChatMessage,
				Console_->GetEntryID (),
				EncodedMessage (message, IMessage::DIn));

		Console_->HandleMessage (msg);
	}

	void IrcServerHandler::InitErrorsReplys ()
	{
		Error2Action_ ["401"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["402"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["403"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["404"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["405"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["406"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["407"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["408"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["409"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["411"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["412"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["413"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["414"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["415"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["421"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["422"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["424"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["431"] =
				boost::bind (&IrcServerHandler::NickCmdError, this);
		Error2Action_ ["432"] =
				boost::bind (&IrcServerHandler::NickCmdError, this);
		Error2Action_ ["433"] =
				boost::bind (&IrcServerHandler::NickCmdError, this);
		Error2Action_ ["436"] =
				boost::bind (&IrcServerHandler::NickCmdError, this);
		Error2Action_ ["437"] =
				boost::bind (&IrcServerHandler::NickCmdError, this);
		Error2Action_ ["441"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["442"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["443"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["444"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["445"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["446"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["451"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["461"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["462"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["463"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["464"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["465"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["466"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["467"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["471"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["472"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["473"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["474"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["475"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["476"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["477"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["478"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["481"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["482"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["483"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["484"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["485"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["491"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["501"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["502"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
	}

	void IrcServerHandler::InitCommandResponses ()
	{
		Command2Action_ ["005"] =
				boost::bind (&IrcServerHandler::SetISupport,
					 this, _1, _2, _3);
		Command2Action_ ["332"] =
				boost::bind (&IrcServerHandler::SetTopic,
					this, _1, _2, _3);
		Command2Action_ ["topic"] =
				boost::bind (&IrcServerHandler::SetTopic,
					this, _1, _2, _3);
		Command2Action_ ["376"] =
				boost::bind (&IrcServerHandler::JoinFromQueue,
					this, _1, _2, _3);
		Command2Action_ ["353"] =
				boost::bind (&IrcServerHandler::AddParticipants,
					this, _1, _2, _3);
		Command2Action_ ["join"] =
				boost::bind (&IrcServerHandler::JoinParticipant,
					this, _1, _2, _3);
		Command2Action_ ["part"] =
				boost::bind (&IrcServerHandler::LeaveParticipant,
					this, _1, _2, _3);
		Command2Action_ ["privmsg"] =
				boost::bind (&IrcServerHandler::HandleIncomingMessage,
					this, _1, _2, _3);
		Command2Action_ ["ping"] =
				boost::bind (&IrcServerHandler::PongMessage,
					this, _1, _2, _3);
		Command2Action_ ["nick"] =
				boost::bind (&IrcServerHandler::ChangeNickname,
					this, _1, _2, _3);
		Command2Action_ ["ctcp_rpl"] =
				boost::bind (&IrcServerHandler::CTCPReply,
					this, _1, _2, _3);
		Command2Action_ ["ctcp_rqst"] =
				boost::bind (&IrcServerHandler::CTCPRequestResult,
					 this, _1, _2, _3);
		Command2Action_ ["invite"] =
				boost::bind (&IrcServerHandler::InviteToChannel,
					 this, _1, _2, _3);
		Command2Action_ ["kick"] =
				boost::bind (&IrcServerHandler::KickFromChannel,
					 this, _1, _2, _3);


		Name2Command_ ["nick"] = boost::bind (&IrcParser::NickCommand,
				IrcParser_, _1);
		Name2Command_ ["quote"] = boost::bind (&IrcParser::RawCommand,
				IrcParser_, _1);
		Name2Command_ ["ctcp"] = boost::bind (&IrcParser::CTCPRequest,
				IrcParser_, _1);
		Name2Command_ ["topic"] = boost::bind (&IrcParser::TopicCommand,
				IrcParser_, _1);
		Name2Command_ ["names"] = boost::bind (&IrcParser::NamesCommand,
				IrcParser_, _1);
		Name2Command_ ["invite"] =
				boost::bind (&IrcParser::InviteCommand, IrcParser_, _1);
		Name2Command_ ["quit"] =
				boost::bind (&ClientConnection::QuitServer,
						Account_->GetClientConnection ().get (),
						_1);
		Name2Command_ ["join"] =
				boost::bind (&IrcServerHandler::JoinChannelByCmd, this,
						_1);
		Name2Command_ ["part"] =
				boost::bind (&IrcParser::PartCommand, IrcParser_, _1);
		Name2Command_ ["kick"] = boost::bind (&IrcParser::KickCommand,
				IrcParser_, _1);
		Name2Command_ ["me"] = boost::bind (&IrcParser::CTCPRequest,
				IrcParser_, _1);
		Name2Command_ ["oper"] = boost::bind (&IrcParser::OperCommand,
				IrcParser_, _1);
		Name2Command_ ["squit"] = boost::bind (&IrcParser::SQuitCommand,
				IrcParser_, _1);
		Name2Command_ ["motd"] = boost::bind (&IrcParser::MOTDCommand,
				IrcParser_, _1);
		Name2Command_ ["lusers"] =
				boost::bind (&IrcParser::LusersCommand, IrcParser_, _1);
		Name2Command_ ["version"] =
				boost::bind (&IrcParser::VersionCommand, IrcParser_, _1);
		Name2Command_ ["stats"] = boost::bind (&IrcParser::StatsCommand,
				IrcParser_, _1);
		Name2Command_ ["links"] = boost::bind (&IrcParser::LinksCommand,
				IrcParser_, _1);
		Name2Command_ ["time"] = boost::bind (&IrcParser::TimeCommand,
				IrcParser_, _1);
		Name2Command_ ["connect"] =
				boost::bind (&IrcParser::ConnectCommand, IrcParser_, _1);
	}

	void IrcServerHandler::NoSuchNickError ()
	{
	}

	void IrcServerHandler::NickCmdError ()
	{
		int index = Account_->GetNickNames ().indexOf (NickName_);
		if (index < Account_->GetNickNames ().count ())
		{
			NickName_ = Account_->GetNickNames ().at (++index);
			IrcParser_->NickCommand (QStringList () << NickName_);
		}
	}

	QString IrcServerHandler::EncodedMessage (const QString& msg,
			IMessage::Direction dir)
	{
		QTextCodec *codec = QTextCodec::codecForName (ServerOptions_
				.ServerEncoding_.toUtf8 ());
		if (dir == IMessage::DIn)
			return codec->toUnicode (msg.toAscii ());

		return codec->fromUnicode (msg);
	}

	ServerParticipantEntry_ptr IrcServerHandler::GetParticipantEntry
			(const QString& nick)
	{
		if (Nick2Entry_.contains (nick))
			return Nick2Entry_ [nick];
		ServerParticipantEntry_ptr entry (CreateParticipantEntry (nick));
		Nick2Entry_ [nick] = entry;
		return entry;
	}

	void IrcServerHandler::RemoveParticipantEntry (const QString& nick)
	{
		//TODO leave from server
		Nick2Entry_.remove (nick);
	}

	void IrcServerHandler::UnregisterChannel (ChannelHandler* ich)
	{
		ChannelHandlers_.remove (ich->GetChannelID ());
		if (!ChannelHandlers_.count () && !Nick2Entry_.count () &&
			XmlSettingsManager::Instance ()
					.property ("AutoDisconnectFromServer").toBool ())
				Account_->GetClientConnection ()->
						CloseServer (ServerID_);
	}

	ServerParticipantEntry_ptr IrcServerHandler::CreateParticipantEntry
			(const QString& nick)
	{
		ServerParticipantEntry_ptr entry
				(new ServerParticipantEntry (nick, ServerID_, Account_));
		Account_->handleGotRosterItems (QList<QObject*> ()
				<< entry.get ());
		return entry;
	}

	void IrcServerHandler::JoinFromQueue (const QString&,
			const QList<std::string>&, const QString&)
	{
		Q_FOREACH (const ChannelOptions& co, ChannelsQueue_)
		{
			JoinChannel (co);
			ChannelsQueue_.removeAll (co);
		}
	}

	void IrcServerHandler::SetTopic (const QString&,
			const QList<std::string>& params, const QString& message)
	{
		QString channelId =
				(QString::fromUtf8 (params.last ().c_str ()) +
				"@" + ServerOptions_.ServerName_).toLower ();

		if (ChannelHandlers_.contains (channelId))
		{
			ChannelHandlers_ [channelId]->
					SetMUCSubject (EncodedMessage (message,
						IMessage::DIn));
		}
	}

	void IrcServerHandler::AddParticipants (const QString&,
			const QList<std::string>& params, const QString& message)
	{
		QString channelID = (QString::fromUtf8 (params.last ().c_str ())
				+ "@" + ServerOptions_.ServerName_).toLower ();
		QStringList participants = message.split (' ');

		if (!ChannelHandlers_ [channelID]->IsRosterReceived ())
		{
			Q_FOREACH (QString nick, participants)
				ChannelHandlers_ [channelID]->SetChannelUser (nick);

			ChannelHandlers_ [channelID]->SetRosterReceived (true);
		}
		else
		{
			QString cmd = "names " +
					QString::fromUtf8 (params.last ().c_str ());

			Q_FOREACH (ChannelHandler *ich, ChannelHandlers_.values ())
				ich->ShowServiceMessage (message,
						IMessage::MTEventMessage,
						IMessage::MSTOther);
		}
	}

	void IrcServerHandler::JoinParticipant (const QString& nick,
			const QList<std::string>&, const QString& msg)
	{
		if (nick == NickName_)
		{
			ChannelJoined_ = true;
			return;
		}
		QString channelID = (msg + "@" + ServerOptions_.ServerName_)
				.toLower ();

		ChannelHandlers_ [channelID]->SetChannelUser (nick);
	}

	void IrcServerHandler::LeaveParticipant (const QString& nick,
			const QList<std::string>& params, const QString& msg)
	{
		QString channelID = (QString::fromUtf8 (params.last ().c_str ())
				+ "@" + ServerOptions_.ServerName_).toLower ();
		if (nick == NickName_)
			ChannelHandlers_ [channelID]->LeaveChannel (msg, false);
		else
			ChannelHandlers_ [channelID]->RemoveChannelUser (nick
					, msg
					, 0);
	}

	void IrcServerHandler::HandleIncomingMessage (const QString& nick,
			const QList<std::string>& params, const QString& msg)
	{
		QString target = QString::fromUtf8 (params.last ().c_str ());

		if (target.startsWith ("#") || target.startsWith ("+") ||
				target.startsWith ("!") || target.startsWith ("&") ||
				target.startsWith ("$"))
		{
				QString channelKey = (target + "@" +
						ServerOptions_.ServerName_).toLower ();
				if (ChannelHandlers_.contains (channelKey))
					ChannelHandlers_ [channelKey]->
							HandleIncomingMessage (nick,
								EncodedMessage (msg, IMessage::DIn));
		}
		else
		{
			ServerParticipantEntry_ptr entry =
					GetParticipantEntry (nick);
			IrcMessage *message =
					new IrcMessage (IMessage::MTChatMessage,
							IMessage::DIn,
							ServerID_,
							nick,
							Account_->GetClientConnection ().get ());
			message->SetBody (EncodedMessage (msg, IMessage::DIn));
			message->SetDateTime (QDateTime::currentDateTime ());
			entry->SetPrivateChat (true);
			entry->HandleMessage (message);
		}
	}

	void IrcServerHandler::PongMessage (const QString&,
			const QList<std::string>&, const QString& msg)
	{
		IrcParser_->PongCommand (msg);
	}

	void IrcServerHandler::SetISupport (const QString&,
			const QList<std::string>& params, const QString&)
	{
		Q_FOREACH (std::string str, params)
		{
			QString string = QString::fromUtf8 (str.c_str ());
			if (string.startsWith ('-') &&
					ISupport_.contains (string.mid (1)))
				ISupport_.remove (string.mid (1));
			else if (!string.contains ('='))
				ISupport_ [string] = true;
			else
			{
				QString key = string.left (string.indexOf ('='));
				QString value = string.mid (string.indexOf ('=') + 1);
				ISupport_ [key] = value;
			}
		}
	}

	void IrcServerHandler::ChangeNickname (const QString& nick,
			const QList<std::string>&, const QString& msg)
	{
		Q_FOREACH (const QString& channel,
				Nick2Entry_ [nick]->GetChannels ())
		{
			QString id = (channel + "@" + ServerOptions_.ServerName_)
					.toLower ();
			QString mess =
					tr ("%1 changed nickname to %2").arg (nick, msg);
			if (ChannelHandlers_.contains (id))
				ChannelHandlers_ [id]->ShowServiceMessage (mess,
						IMessage::MTStatusMessage,
						IMessage::MSTParticipantNickChange);
		}

		Account_->handleEntryRemoved (Nick2Entry_ [nick].get ());

		ServerParticipantEntry_ptr entry = Nick2Entry_.take (nick);

		entry->SetEntryName (msg);
		Account_->handleGotRosterItems (QList<QObject*> ()
				<< entry.get ());

		Nick2Entry_ [msg] = entry;
		if (nick == NickName_)
			NickName_ = msg;
	}

	void IrcServerHandler::CTCPReply (const QString& nick,
			const QList<std::string>& params, const QString& msg)
	{
		QString cmd;
		QString outputMessage;
		QString version = QString ("%1 %2 %3").arg ("Acetamide",
				"2.0",
				"(C) 2011 by the LeechCraft team");
		QDateTime currentDT = QDateTime::currentDateTime ();
		QString mess = msg.mid (1, msg.length () - 2);
		QStringList ctcpList = mess.split (' ');
		QString firstPartOutput = QString ("%1 %2 - %3").arg ("Acetamide"
				, "2.0"
				, "http://www.leechcraft.org");

		if (ctcpList.at (0).toLower () == "version")
		{
			cmd = QString ("%1 %2%3").arg ("\001VERSION"
					, version
					, QChar ('\001'));
			outputMessage = tr ("Received request %1 from %2,"
					" sending response").arg ("VERSION", nick);
		}
		else if (ctcpList.at (0).toLower () == "ping")
		{
			cmd = QString ("%1 %2%3").arg ("\001PING "
					, QString::number (currentDT.toTime_t ())
					, QChar ('\001'));
			outputMessage = tr ("Received request %1 from %2,"
					" sending response").arg ("PING", nick);
		}
		else if (ctcpList.at (0).toLower () == "time")
		{
			cmd = QString ("%1 %2%3").arg ("\001TIME"
					, currentDT.toString ("ddd MMM dd hh:mm:ss yyyy")
					, QChar ('\001'));
			outputMessage = tr ("Received request %1 from %2,"
					" sending response").arg ("TIME", nick);
		}
		else if (ctcpList.at (0).toLower () == "source")
		{
			cmd = QString ("%1 %2 %3").arg ("\001SOURCE"
					, firstPartOutput
					, QChar ('\001'));
			outputMessage = tr ("Received request %1 from %2,"
					" sending response").arg ("SOURCE", nick);
		}
		else if (ctcpList.at (0).toLower () == "clientinfo")
		{
			cmd = QString ("%1 %2 - %3 %4 %5").arg ("\001CLIENTINFO"
					, firstPartOutput
					, "Supported tags:"
					, "VERSION PING TIME SOURCE CLIENTINFO"
					, QChar ('\001'));
			outputMessage = tr ("Received request %1 from %2,"
					" sending response").arg ("CLIENTINFO", nick);
		}
		else if (ctcpList.at (0).toLower () == "action")
		{
			QString mess = "/me " + QStringList (ctcpList.mid (1))
					.join (" ");
			HandleIncomingMessage (nick, params, mess);
			return;
		}
		else
			outputMessage.clear ();

		if (outputMessage.isEmpty ())
			return;

		Q_FOREACH (ChannelHandler *ich, ChannelHandlers_.values ())
			ich->ShowServiceMessage (outputMessage,
					IMessage::MTEventMessage,
					IMessage::MSTOther);

		IrcParser_->CTCPReply (QStringList () << nick << cmd);
	}

	void IrcServerHandler::CTCPRequestResult (const QString& nick, const
			QList<std::string>& params, const QString& msg)
	{
		if (QString::fromUtf8 (params.first ().c_str ()) != NickName_)
			return;
		QString mess = msg.mid (1, msg.length () - 2);
		QStringList ctcpList = mess.split (' ');

		QString output = tr ("Received answer CTCP-%1 from %2: %3")
				.arg (ctcpList.at (0), nick,
					(QStringList (ctcpList.mid (1)))
						.join (" "));

		QString cmd = "ctcp " + nick + " " + ctcpList.at (0).toLower ();
		Q_FOREACH (ChannelHandler *ich, ChannelHandlers_.values ())
			ich->ShowServiceMessage (output,
					IMessage::MTEventMessage,
					IMessage::MSTOther);
	}

	void IrcServerHandler::InviteToChannel (const QString& nick,
			const QList<std::string>& , const QString& msg)
	{
		if (XmlSettingsManager::Instance ()
				.property ("ShowInviteDialog").toBool ())
			XmlSettingsManager::Instance ()
					.setProperty ("InviteActionByDefault", 0);

		if (!XmlSettingsManager::Instance ()
				.property ("InviteActionByDefault").toInt ())
		{
			if (IsInviteDialogActive_)
				InviteChannelsDialog_->AddInvitation (msg, nick);
			else
			{
				std::auto_ptr<InviteChannelsDialog> dic
						(new InviteChannelsDialog (msg, nick, 0));
				IsInviteDialogActive_ = true;
				InviteChannelsDialog_ = dic;
				InviteChannelsDialog_->setModal (true);

				connect (InviteChannelsDialog_.get (),
						SIGNAL (accepted ()),
						this,
						SLOT (joinAfterInvite ()));
			}
			InviteChannelsDialog_->show ();
		}
		else if (XmlSettingsManager::Instance ()
				.property ("InviteActionByDefault").toInt () == 1)
		{
			ChannelOptions co;
			co.ChannelName_ = msg;
			co.ChannelPassword_ = QString ();
			co.ServerName_ = ServerOptions_.ServerName_;
			JoinChannel (co);
		}

		QString outputMessage = nick + tr (" invites you to a channel ")
				+ msg;
		Q_FOREACH (ChannelHandler *ich, ChannelHandlers_.values ())
			ich->ShowServiceMessage (outputMessage,
					IMessage::MTEventMessage,
					IMessage::MSTOther);
	}

	void IrcServerHandler::KickFromChannel (const QString& nick,
			const QList<std::string>& params, const QString& msg)
	{
		QString channelID = (QString::fromUtf8 (params.first ().c_str ())
				+ "@" + ServerOptions_.ServerName_).toLower ();

		ChannelHandlers_ [channelID]->RemoveChannelUser (
				QString::fromUtf8 (params.last ().c_str ())
				, EncodedMessage (msg, IMessage::DIn)
				, 1
				, nick);
	}

	void IrcServerHandler::InitSocket ()
	{
		connect (TcpSocket_ptr.get (),
				SIGNAL (readyRead ()),
				this,
				SLOT (readReply ()));

		connect (TcpSocket_ptr.get (),
				SIGNAL (connected ()),
				this,
				SLOT (connectionEstablished ()));

		connect (TcpSocket_ptr.get (),
				SIGNAL (error (QAbstractSocket::SocketError)),
				Account_->GetClientConnection ().get (),
				SLOT (handleError (QAbstractSocket::SocketError)));
	}

	bool IrcServerHandler::IsErrorReply (const QString& cmd)
	{
		return Error2Action_.contains (cmd);
	}

	bool IrcServerHandler::IsCTCPMessage (const QString& msg)
	{
		return msg.startsWith ('\001') && msg.endsWith ('\001');
	}

	void IrcServerHandler::readReply ()
	{
		while (TcpSocket_ptr->canReadLine ())
		{
			QString str = TcpSocket_ptr->readLine ();
			SendToConsole (str.trimmed ());
			qDebug () << str;
			if (!IrcParser_->ParseMessage (str))
				return;

			QString cmd = IrcParser_->GetIrcMessageOptions ()
					.Command_.toLower ();
			if (IsErrorReply (cmd))
			{
				QString msg = IrcParser_->GetIrcMessageOptions ()
						.Message_ + " " + QString::fromUtf8 (IrcParser_->
							GetIrcMessageOptions ().Parameters_.last ()
								.c_str ());
				Entity e = Util::MakeNotification ("Azoth",
						msg,
						PInfo_);
				Core::Instance ().SendEntity (e);
				Error2Action_ [cmd] ();
			}
			else if ((cmd != "join") && (!ChannelJoined_))
				IncomingMessage2Server ();

			IncomingMessage2Channel ();
		}
	}

	void IrcServerHandler::connectionEstablished ()
	{
		ServerConnectionState_ = Connected;
		emit connected (ServerID_);
		if (XmlSettingsManager::Instance ().property
			("ServerConsole")
			.toBool ())
		{
				Console_.reset (new IrcServerConsole (this, Account_));
				Account_->handleGotRosterItems (QList<QObject*> () <<
						Console_.get ());
				IsConsoleEnabled_ = true;
				Console_->SetStatus (EntryStatus (SOnline, QString ()));
		}

		ServerCLEntry_->SetStatus (EntryStatus (SOnline, QString ()));
		IrcParser_->AuthCommand ();
	}

	void IrcServerHandler::joinAfterInvite ()
	{
		Q_FOREACH (const QString& channel,
				InviteChannelsDialog_->GetChannels ())
		{
			ChannelOptions co;
			co.ChannelName_ = channel;
			co.ChannelPassword_ = QString ();
			co.ServerName_ = ServerOptions_.ServerName_;
			JoinChannel (co);
		}
	}

};
};
};
