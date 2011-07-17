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

#include "ircserverhandler.h"
#include <boost/bind.hpp>
#include <QTextCodec>
#include <QMessageBox>
#include <QInputDialog>
#include <util/util.h>
#include <util/notificationactionhandler.h>
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

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcServerHandler::IrcServerHandler (const ServerOptions& server,
			IrcAccount *account )
	: Account_ (account)
	, ErrorHandler_ (new IrcErrorHandler (this))
	, IrcParser_ (0)
	, ServerCLEntry_ (new IrcServerCLEntry (this, account))
	, ServerConnectionState_ (NotConnected)
	, IsConsoleEnabled_ (false)
	, IsInviteDialogActive_ (false)
	, ServerID_ (server.ServerName_ + ":" +
			QString::number (server.ServerPort_))
	, NickName_ (server.ServerNickName_)
	, ServerOptions_ (server)
	{
		IrcParser_ = new IrcParser (this);
		InitCommandResponses ();
		connect (this,
				SIGNAL (connected (const QString&)),
				Account_->GetClientConnection ().get (),
				SLOT (serverConnected (const QString&)));

		connect (this,
				SIGNAL (disconnected (const QString&)),
				Account_->GetClientConnection ().get (),
				SLOT (serverDisconnected (const QString&)));

		connect (this,
				SIGNAL (nicknameConflict (const QString&)),
				ServerCLEntry_,
				SIGNAL (nicknameConflict (const QString&)));
	}

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

	QString IrcServerHandler::GetNickName () const
	{
		return NickName_;
	}

	void IrcServerHandler::SetNickName (const QString& nick)
	{
		NickName_ = nick;
	}

	QString IrcServerHandler::GetServerID_ () const
	{
		return ServerID_;
	}

	ServerOptions IrcServerHandler::GetServerOptions () const
	{
		return ServerOptions_;
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
		LastSendId_ = channelId;
		IrcParser_->PrivMsgCommand (EncodedMessage (msg, IMessage::DOut),
				ChannelHandlers_ [channelId]->GetChannelOptions ().ChannelName_);
	}

	void IrcServerHandler::SendPrivateMessage (IrcMessage* msg)
	{
		LastSendId_ = msg->GetOtherVariant ();
		IrcParser_->PrivMsgCommand (EncodedMessage (msg->GetBody (), IMessage::DOut),
				msg->GetOtherVariant ());
	}

	void IrcServerHandler::SendCommandMessage2Server (const QString& msg)
	{
		QString mess = msg;
		if (msg.startsWith ('/'))
			mess = msg.mid (1);
		LastSendId_ = QString ();
		QString commandMessage = EncodedMessage (mess, IMessage::DOut);
		QStringList commandWithParams = commandMessage.split (' ');
		if (Name2Command_.contains (commandWithParams.at (0).toLower ()))
			Name2Command_ [commandWithParams.at (0).toLower ()] (commandWithParams.mid (1));
		else
			IrcParser_->RawCommand (commandWithParams);

		IrcMessage *mesg = CreateMessage (IMessage::MTEventMessage,
				ServerID_, EncodedMessage (mess, IMessage::DIn));
		ServerCLEntry_->HandleMessage (mesg);
	}

	void IrcServerHandler::ParseMessageForCommand (const QString& msg,
			const QString& channelID)
	{
		LastSendId_ = channelID;
		QString commandMessage = EncodedMessage (msg.mid (1), IMessage::DOut);
		QStringList commandWithParams = commandMessage.split (' ');
		if (Name2Command_.contains (commandWithParams.at (0).toLower ()))
		{
			if (commandWithParams.at (0).toLower () == "quit")
				commandWithParams.append (ServerID_);
			else if (commandWithParams.at (0).toLower () == "me")
			{
				commandWithParams.insert (1, channelID.left (channelID.indexOf ('@')));
				commandWithParams.insert (2, "ACTION");
			}
			Name2Command_ [commandWithParams.at (0).toLower ()] (commandWithParams.mid (1));
		}
		else
			IrcParser_->RawCommand (commandWithParams);
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

	void IrcServerHandler::LeaveChannel (const QString& channel,
			const QString& msg)
	{
		IrcParser_->PartCommand (QStringList () << channel 
				<< QString (" :" + msg));
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
				Account_->GetClientConnection ()->CloseServer (ServerID_);
		}
	}

	ChannelHandler* IrcServerHandler::GetChannelHandler (const QString& id)
	{
		return ChannelHandlers_.contains (id) ?
				ChannelHandlers_ [id] :
				0;
	}

	QList<ServerParticipantEntry_ptr> IrcServerHandler::GetParticipants (const QString& channel)
	{
		QList<ServerParticipantEntry_ptr> result;
		Q_FOREACH (ServerParticipantEntry_ptr spe, Nick2Entry_.values ())
			if (spe->GetChannels ().contains (channel))
				result << spe;
		return result;
	}

	IrcMessage* IrcServerHandler::CreateMessage (IMessage::MessageType type,
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
		if (ServerConnectionState_ != NotConnected)
			return;

		TcpSocket_ptr.reset (new QTcpSocket (this));
		InitSocket ();
		ServerConnectionState_ = InProgress;
		TcpSocket_ptr->connectToHost (ServerOptions_.ServerName_,
				ServerOptions_.ServerPort_);
	}

	void IrcServerHandler::DisconnectFromServer ()
	{
		LeaveAllChannel ();
		CloseAllPrivateChats ();
		if (ServerConnectionState_ != NotConnected)
			TcpSocket_ptr->disconnectFromHost ();
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
		SendToConsole (IMessage::DOut, cmd.trimmed ());

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
		Q_FOREACH (std::string str, IrcParser_->GetIrcMessageOptions ().Parameters_)
			message.append (QString::fromUtf8 (str.c_str ())).append (' ');
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

	void IrcServerHandler::IncomingMessage2Channel (const QString& channelID)
	{
		QString message;
		Q_FOREACH (std::string str, IrcParser_->GetIrcMessageOptions ()
				.Parameters_)
			message.append (QString::fromUtf8 (str.c_str ()))
				.append (' ');
		message.append (IrcParser_->GetIrcMessageOptions ().Message_);

		ChannelPublicMessage *msg =
				new ChannelPublicMessage (EncodedMessage (message, IMessage::DIn),
						IMessage::DIn,
						ChannelHandlers_ [channelID]->GetCLEntry (),
						IMessage::MTEventMessage,
						IMessage::MSTOther);
		ChannelHandlers_ [channelID]->GetCLEntry ()->HandleMessage (msg);
	}

	void IrcServerHandler::SendToConsole (IMessage::Direction dir,
			const QString& message)
	{
		if (!IsConsoleEnabled_)
			return;

		if (dir == IMessage::DIn)
			emit sendMessageToConsole (dir, 
					EncodedMessage (message, dir));
		else
			emit sendMessageToConsole (dir, 
					EncodedMessage (message, IMessage::DIn));
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
		Command2Action_ ["353"] =
				boost::bind (&IrcServerHandler::AddParticipants,
					this, _1, _2, _3);
		Command2Action_ ["join"] =
				boost::bind (&IrcServerHandler::JoinParticipant,
					this, _1, _2, _3);
		Command2Action_ ["part"] =
				boost::bind (&IrcServerHandler::LeaveParticipant,
					this, _1, _2, _3);
		Command2Action_ ["quit"] =
				boost::bind (&IrcServerHandler::QuitParticipant,
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
		Command2Action_ ["302"] =
				boost::bind (&IrcServerHandler::GetUserHost,
					this, _1, _2, _3);
		Command2Action_ ["303"] =
				boost::bind (&IrcServerHandler::GetIson,
					this, _1, _2, _3);
		Command2Action_ ["305"] =
				boost::bind (&IrcServerHandler::GetAway,
					this, _1, _2, _3);
		Command2Action_ ["306"] =
				boost::bind (&IrcServerHandler::GetAway,
					this, _1, _2, _3);
		Command2Action_ ["311"] =
				boost::bind (&IrcServerHandler::GetWhoIsUser,
					this, _1, _2, _3);
		Command2Action_ ["315"] =
				boost::bind (&IrcServerHandler::GetEndMessage,
					this, _1, QList<std::string> () << "who", _3);
		Command2Action_ ["312"] =
				boost::bind (&IrcServerHandler::GetWhoIsServer,
					this, _1, _2, _3);
		Command2Action_ ["313"] =
				boost::bind (&IrcServerHandler::GetWhoIsOperator,
					this, _1, _2, _3);
		Command2Action_ ["317"] =
				boost::bind (&IrcServerHandler::GetWhoIsIdle,
					this, _1, _2, _3);
		Command2Action_ ["318"] =
				boost::bind (&IrcServerHandler::GetEndMessage,
					this, _1, QList<std::string> () << "whois", _3);
		Command2Action_ ["319"] =
				boost::bind (&IrcServerHandler::GetWhoIsChannels,
					this, _1, _2, _3);
		Command2Action_ ["314"] =
				boost::bind (&IrcServerHandler::GetWhoWas,
					this, _1, _2, _3);
		Command2Action_ ["366"] =
				boost::bind (&IrcServerHandler::GetEndMessage,
					this, _1, QList<std::string> () << "whowas", _3);
		Command2Action_ ["331"] =
				boost::bind (&IrcServerHandler::GetNoTopic,
					this, _1, _2, _3);
		Command2Action_ ["341"] =
				boost::bind (&IrcServerHandler::GetInviting,
					this, _1, _2, _3);
		Command2Action_ ["342"] =
				boost::bind (&IrcServerHandler::GetSummoning,
					this, _1, _2, _3);
		Command2Action_ ["351"] =
				boost::bind (&IrcServerHandler::GetVersion,
					this, _1, _2, _3);
		Command2Action_ ["352"] =
				boost::bind (&IrcServerHandler::GetWho,
					this, _1, _2, _3);
		Command2Action_ ["366"] =
				boost::bind (&IrcServerHandler::GetEndMessage,
					this, _1, QList<std::string> () << "names", _3);
		Command2Action_ ["364"] =
				boost::bind (&IrcServerHandler::GetLinks,
					this, _1, _2, _3);
		Command2Action_ ["365"] =
				boost::bind (&IrcServerHandler::GetEndMessage,
					this, _1, QList<std::string> () << "links", _3);
		Command2Action_ ["371"] =
				boost::bind (&IrcServerHandler::GetInfo,
					this, _1, _2, _3);
		Command2Action_ ["374"] =
				boost::bind (&IrcServerHandler::GetEndMessage,
					this, _1, QList<std::string> () << "info", _3);
		Command2Action_ ["372"] =
				boost::bind (&IrcServerHandler::GetMotd,
					this, _1, _2, _3);
		Command2Action_ ["375"] =
				boost::bind (&IrcServerHandler::GetMotd,
					this, _1, _2, _3);
		Command2Action_ ["376"] =
				boost::bind (&IrcServerHandler::GetEndMessage,
					this, _1, QList<std::string> () << "motd", _3);
		Command2Action_ ["381"] =
				boost::bind (&IrcServerHandler::GetYoureOper,
					this, _1, _2, _3);
		Command2Action_ ["382"] =
				boost::bind (&IrcServerHandler::GetRehash,
					this, _1, _2, _3);
		Command2Action_ ["391"] =
				boost::bind (&IrcServerHandler::GetTime,
					this, _1, _2, _3);
		Command2Action_ ["392"] =
				boost::bind (&IrcServerHandler::GetUsersStart,
					this, _1, _2, _3);
		Command2Action_ ["393"] =
				boost::bind (&IrcServerHandler::GetUsers,
					this, _1, _2, _3);
		Command2Action_ ["394"] =
				boost::bind (&IrcServerHandler::GetEndMessage,
					this, _1, QList<std::string> () << "users", _3);
		Command2Action_ ["395"] =
				boost::bind (&IrcServerHandler::GetNoUser,
					this, _1, _2, _3);
		Command2Action_ ["200"] =
				boost::bind (&IrcServerHandler::GetTraceLink,
					this, _1, _2, _3);
		Command2Action_ ["201"] =
				boost::bind (&IrcServerHandler::GetTraceConnecting,
					this, _1, _2, _3);
		Command2Action_ ["202"] =
				boost::bind (&IrcServerHandler::GetTraceHandshake,
					this, _1, _2, _3);
		Command2Action_ ["203"] =
				boost::bind (&IrcServerHandler::GetTraceUnknown,
					this, _1, _2, _3);
		Command2Action_ ["204"] =
				boost::bind (&IrcServerHandler::GetTraceOperator,
					this, _1, _2, _3);
		Command2Action_ ["205"] =
				boost::bind (&IrcServerHandler::GetTraceUser,
					this, _1, _2, _3);
		Command2Action_ ["206"] =
				boost::bind (&IrcServerHandler::GetTraceServer,
					this, _1, _2, _3);
		Command2Action_ ["207"] =
				boost::bind (&IrcServerHandler::GetTraceService,
					this, _1, _2, _3);
		Command2Action_ ["208"] =
				boost::bind (&IrcServerHandler::GetTraceNewType,
					this, _1, _2, _3);
		Command2Action_ ["209"] =
				boost::bind (&IrcServerHandler::GetTraceClass,
					this, _1, _2, _3);
		Command2Action_ ["261"] =
				boost::bind (&IrcServerHandler::GetTraceLog,
					this, _1, _2, _3);
		Command2Action_ ["262"] =
				boost::bind (&IrcServerHandler::GetTraceEnd,
					this, _1, _2, _3);
		Command2Action_ ["211"] =
				boost::bind (&IrcServerHandler::GetStatsLinkInfo,
					this, _1, _2, _3);
		Command2Action_ ["212"] =
				boost::bind (&IrcServerHandler::GetStatsCommands,
					this, _1, _2, _3);
		Command2Action_ ["219"] =
				boost::bind (&IrcServerHandler::GetStatsEnd,
					this, _1, _2, _3);
		Command2Action_ ["242"] =
				boost::bind (&IrcServerHandler::GetStatsUptime,
					this, _1, _2, _3);
		Command2Action_ ["243"] =
				boost::bind (&IrcServerHandler::GetStatsOline,
					this, _1, _2, _3);
		Command2Action_ ["251"] =
				boost::bind (&IrcServerHandler::GetLuserClient,
					this, _1, _2, _3);
		Command2Action_ ["252"] =
				boost::bind (&IrcServerHandler::GetLuserOp,
					this, _1, _2, _3);
		Command2Action_ ["253"] =
				boost::bind (&IrcServerHandler::GetLuserUnknown,
					this, _1, _2, _3);
		Command2Action_ ["254"] =
				boost::bind (&IrcServerHandler::GetLuserChannels,
					this, _1, _2, _3);
		Command2Action_ ["255"] =
				boost::bind (&IrcServerHandler::GetLuserMe,
					this, _1, _2, _3);
		Command2Action_ ["256"] =
				boost::bind (&IrcServerHandler::GetAdmineMe,
					this, _1, _2, _3);
		Command2Action_ ["257"] =
				boost::bind (&IrcServerHandler::GetAdminLoc1,
					this, _1, _2, _3);
		Command2Action_ ["258"] =
				boost::bind (&IrcServerHandler::GetAdminLoc2,
					this, _1, _2, _3);
		Command2Action_ ["259"] =
				boost::bind (&IrcServerHandler::GetAdminEmail,
					this, _1, _2, _3);
		Command2Action_ ["263"] =
				boost::bind (&IrcServerHandler::GetTryAgain,
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
		Name2Command_ ["trace"] = boost::bind (&IrcParser::TraceCommand,
				IrcParser_, _1);
		Name2Command_ ["admin"] = boost::bind (&IrcParser::AdminCommand,
				IrcParser_, _1);
		Name2Command_ ["info"] = boost::bind (&IrcParser::InfoCommand,
				IrcParser_, _1);
		Name2Command_ ["who"] = boost::bind (&IrcParser::WhoCommand,
				IrcParser_, _1);
		Name2Command_ ["whois"] = boost::bind (&IrcParser::WhoisCommand,
				IrcParser_, _1);
		Name2Command_ ["whowas"] =
				boost::bind (&IrcParser::WhowasCommand, IrcParser_, _1);
		Name2Command_ ["kill"] = boost::bind (&IrcParser::KillCommand,
				IrcParser_, _1);
		Name2Command_ ["ping"] = boost::bind (&IrcParser::PingCommand,
				IrcParser_, _1);
		Name2Command_ ["pong"] = boost::bind (&IrcParser::PongCommand,
				IrcParser_, _1);
		Name2Command_ ["away"] = boost::bind (&IrcParser::AwayCommand,
				IrcParser_, _1);
		Name2Command_ ["rehash"] =
				boost::bind (&IrcParser::RehashCommand, IrcParser_, _1);
		Name2Command_ ["die"] = boost::bind (&IrcParser::DieCommand,
				IrcParser_, _1);
		Name2Command_ ["restart"] =
				boost::bind (&IrcParser::RestartCommand, IrcParser_, _1);
		Name2Command_ ["summon"] =
				boost::bind (&IrcParser::SummonCommand, IrcParser_, _1);
		Name2Command_ ["users"] =
				boost::bind (&IrcParser::UsersCommand, IrcParser_, _1);
		Name2Command_ ["userhost"] =
				boost::bind (&IrcParser::UserhostCommand,
						IrcParser_, _1);
		Name2Command_ ["wallops"] =
				boost::bind (&IrcParser::WallopsCommand, IrcParser_, _1);
		Name2Command_ ["ison"] =
				boost::bind (&IrcParser::IsonCommand, IrcParser_, _1);
	}

	void IrcServerHandler::NickCmdError ()
	{
		int index = Account_->GetNickNames ().indexOf (NickName_);

		if (index != Account_->GetNickNames ().count () - 1)
			NickName_ = Account_->GetNickNames ().at (++index);
		else
			NickName_ = Account_->GetNickNames ().at (0);

		if (NickName_.isEmpty ())
		{
			NickCmdError ();
			return;
		}

		if (NickName_ == OldNickName_)
		{
			emit nicknameConflict (NickName_);
			return;
		}

		IrcParser_->NickCommand (QStringList () << NickName_);
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

	ServerParticipantEntry_ptr IrcServerHandler::GetParticipantEntry (const QString& nick)
	{
		if (Nick2Entry_.contains (nick))
			return Nick2Entry_ [nick];
		ServerParticipantEntry_ptr entry (CreateParticipantEntry (nick));
		Nick2Entry_ [nick] = entry;
		return entry;
	}

	void IrcServerHandler::RemoveParticipantEntry (const QString& nick)
	{
		Nick2Entry_.remove (nick);
	}

	void IrcServerHandler::UnregisterChannel (ChannelHandler* ich)
	{
		ChannelHandlers_.remove (ich->GetChannelID ());
		if (!ChannelHandlers_.count () && !Nick2Entry_.count () &&
				XmlSettingsManager::Instance ()
						.property ("AutoDisconnectFromServer").toBool ())
			Account_->GetClientConnection ()->CloseServer (ServerID_);
	}

	void IrcServerHandler::SetConsoleEnabled (bool enabled)
	{
		IsConsoleEnabled_ = enabled;
	}

	void IrcServerHandler::LeaveAllChannel ()
	{
		QString msg = QString ();
		Q_FOREACH (ChannelHandler *ch, ChannelHandlers_.values ())
			ch->LeaveChannel (msg);
	}

	void IrcServerHandler::CloseAllPrivateChats ()
	{
		Q_FOREACH (ServerParticipantEntry_ptr spe, Nick2Entry_.values ())
			if (spe->IsPrivateChat ())
				spe->closePrivateChat (true);
	}

	ServerParticipantEntry_ptr IrcServerHandler::CreateParticipantEntry (const QString& nick)
	{
		ServerParticipantEntry_ptr entry (new ServerParticipantEntry (nick, ServerID_, Account_));
		Account_->handleGotRosterItems (QList<QObject*> () << entry.get ());
		return entry;
	}

	void IrcServerHandler::JoinFromQueue ()
	{
		Q_FOREACH (const ChannelOptions& co, ChannelsQueue_)
		{
			JoinChannel (co);
			ChannelsQueue_.removeAll (co);
		}
	}

	void IrcServerHandler::ShowAnswer (const QString& msg)
	{
		if (!LastSendId_.isEmpty ())
			ChannelHandlers_ [LastSendId_]->
					ShowServiceMessage (EncodedMessage (msg, IMessage::DIn),
							IMessage::MTEventMessage,
							IMessage::MSTOther);
		else
			ServerCLEntry_->HandleMessage (CreateMessage (IMessage::MTEventMessage,
					ServerID_, EncodedMessage (msg, IMessage::DIn)));
	}

	void IrcServerHandler::SetTopic (const QString&,
			const QList<std::string>& params, const QString& message)
	{
		QString channelId = (QString::fromUtf8 (params.last ().c_str ()) +
				"@" + ServerOptions_.ServerName_).toLower ();

		if (ChannelHandlers_.contains (channelId))
			ChannelHandlers_ [channelId]->
					SetMUCSubject (EncodedMessage (message, IMessage::DIn));
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
			ShowAnswer (message);
	}

	void IrcServerHandler::JoinParticipant (const QString& nick,
			const QList<std::string>&, const QString& msg)
	{
		if (nick == NickName_)
			return;

		QString channelID = (msg + "@" + ServerOptions_.ServerName_).toLower ();

		if (ChannelHandlers_.contains (channelID))
			ChannelHandlers_ [channelID]->SetChannelUser (nick);
	}

	void IrcServerHandler::LeaveParticipant (const QString& nick,
			const QList<std::string>& params, const QString& msg)
	{
		QString channelID = (QString::fromUtf8 (params.last ().c_str ())
				+ "@" + ServerOptions_.ServerName_).toLower ();

		if (nick == NickName_)
			ChannelHandlers_ [channelID]->LeaveChannel (msg);
		else
			ChannelHandlers_ [channelID]->RemoveChannelUser (nick,
					EncodedMessage (msg, IMessage::DIn), 0);
	}

	void IrcServerHandler::QuitParticipant (const QString& nick,
			const QList<std::string>&, const QString& msg)
	{
		if (nick == NickName_)
			Account_->GetClientConnection ()->QuitServer (QStringList () << ServerID_);
		else
			if (Nick2Entry_.contains (nick))
				Q_FOREACH (const QString& channel, Nick2Entry_ [nick]->GetChannels ())
				{
					QString channelID = channel + "@" + ServerOptions_.ServerName_;
					ChannelHandlers_ [channelID]->RemoveChannelUser (nick,
							EncodedMessage (msg, IMessage::DIn), 0);
				}
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
		IrcParser_->PongCommand (QStringList () << msg);
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
		Q_FOREACH (const QString& channel, Nick2Entry_ [nick]->GetChannels ())
		{
			QString id = (channel + "@" + ServerOptions_.ServerName_).toLower ();
			QString mess = tr ("%1 changed nickname to %2")
					.arg (nick, EncodedMessage (msg, IMessage::DIn));

			if (ChannelHandlers_.contains (id))
				ChannelHandlers_ [id]->ShowServiceMessage (mess,
						IMessage::MTStatusMessage,
						IMessage::MSTParticipantNickChange);
		}

		Account_->handleEntryRemoved (Nick2Entry_ [nick].get ());
		ServerParticipantEntry_ptr entry = Nick2Entry_.take (nick);
		entry->SetEntryName (msg);
		Account_->handleGotRosterItems (QList<QObject*> () << entry.get ());
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

		ShowAnswer (nick + tr (" invites you to a channel ") + msg);
	}

	void IrcServerHandler::KickFromChannel (const QString& nick,
			const QList<std::string>& params, const QString& msg)
	{
		QString channelID = (QString::fromUtf8 (params.first ().c_str ()) + 
				"@" + ServerOptions_.ServerName_).toLower ();

		ChannelHandlers_ [channelID]->
				RemoveChannelUser (QString::fromUtf8 (params.last ().c_str ()), 
						EncodedMessage (msg, IMessage::DIn), 
						1,
						nick);
	}

	void IrcServerHandler::GetUserHost (const QString&,
			const QList<std::string>&, const QString& msg)
	{
		QStringList params = msg.split (' ');
		Q_FOREACH (const QString& param, params)
			if (!param.isEmpty ())
			{
				int pos = param.indexOf ('=');
				ShowAnswer (param.left (pos) + tr (" is a ") + 
						param.mid (pos + 1));
			}
	}

	void IrcServerHandler::GetIson (const QString&,
			const QList<std::string>&, const QString& msg)
	{
		QStringList list = msg.split (' ');
		Q_FOREACH (const QString& nick, list)
			if (!nick.isEmpty ())
				ShowAnswer (nick + tr (" is online"));
	}

	void IrcServerHandler::GetAway (const QString&,
			const QList<std::string>& , const QString& msg)
	{
		ShowAnswer (msg);
	}

	void IrcServerHandler::GetWhoIsUser (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		QString message = QString::fromUtf8 (params.at (1).c_str ()) +
				" - " + QString::fromUtf8 (params.at (2).c_str ()) + "@"
				+ QString::fromUtf8 (params.at (3).c_str ()) +
				" (" + msg + ")";
		ShowAnswer (message);
	}

	void IrcServerHandler::GetWhoIsServer (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		QString message = QString::fromUtf8 (params.at (1).c_str ()) +
				tr (" connected via ") +
				QString::fromUtf8 (params.at (2).c_str ()) +
				" (" + msg + ")";
		ShowAnswer (message);
	}

	void IrcServerHandler::GetWhoIsOperator (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		ShowAnswer (QString::fromUtf8 (params.at (1).c_str ()) + " " + msg);
	}

	void IrcServerHandler::GetWhoIsIdle (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		QString message = QString::fromUtf8 (params.at (1).c_str ()) +
				" " + QString::fromUtf8 (params.at (1).c_str ()) +
				" " + msg;
		ShowAnswer (message);
	}

	void IrcServerHandler::GetWhoIsChannels (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		QString message = QString::fromUtf8 (params.at (1).c_str ()) +
				tr (" on the channels : ") + msg;
		ShowAnswer (message);
	}

	void IrcServerHandler::GetWhoWas (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		QString message = QString::fromUtf8 (params.at (1).c_str ()) +
				" - " + QString::fromUtf8 (params.at (2).c_str ()) + "@"
				+ QString::fromUtf8 (params.at (3).c_str ()) +
				" (" + msg + ")";
		ShowAnswer (message);
	}

	void IrcServerHandler::GetNoTopic (const QString&,
			const QList<std::string>&, const QString& msg)
	{
		ShowAnswer (msg);
	}

	void IrcServerHandler::GetInviting (const QString&,
			const QList<std::string>& params, const QString&)
	{
		QString msg = "You invite " +
				QString::fromUtf8 (params.at (1).c_str ()) +
				" to a channel " +
				QString::fromUtf8 (params.at (2).c_str ());
		ShowAnswer (msg);
	}

	void IrcServerHandler::GetSummoning (const QString&,
			const QList<std::string>& params, const QString&)
	{
		ShowAnswer (QString::fromUtf8 (params.at (1).c_str ()) + 
				tr (" summoning to IRC"));
	}

	void IrcServerHandler::GetVersion (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		QStringList list;
		Q_FOREACH (std::string str, params)
			list << QString::fromUtf8 (str.c_str ());
		list << msg;
		ShowAnswer (list.join (" "));
	}

	void IrcServerHandler::GetWho (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		QString message = QString::fromUtf8 (params
				.at (params.count () - 1).c_str ()) +
				" - " + QString::fromUtf8 (params.at (2).c_str ()) + "@"
				+ QString::fromUtf8 (params.at (3).c_str ()) +
				" (" + msg.split (' ').at (1) + ")";
		ShowAnswer (message);
	}

	void IrcServerHandler::GetLinks (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		ShowAnswer (QString::fromUtf8 (params.last ().c_str ()) + " :" + msg);
	}

	void IrcServerHandler::GetInfo (const QString&,
			const QList<std::string>&, const QString& msg)
	{
		ShowAnswer (msg);
	}

	void IrcServerHandler::GetMotd (const QString&,
			const QList<std::string>&, const QString& msg)
	{
		ShowAnswer (msg);
	}

	void IrcServerHandler::GetEndMessage (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		ShowAnswer (msg);
		if (params.first () == "motd")
			JoinFromQueue ();
	}

	void IrcServerHandler::GetYoureOper (const QString&,
			const QList<std::string>& , const QString& msg)
	{
		ShowAnswer (msg);
	}

	void IrcServerHandler::GetRehash (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		ShowAnswer (QString::fromUtf8 (params.last ().c_str ()) + " :" + msg);
	}

	void IrcServerHandler::GetTime (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		ShowAnswer (QString::fromUtf8 (params.last ().c_str ()) + " :" + msg);
	}

	void IrcServerHandler::GetUsersStart (const QString&,
			const QList<std::string>&, const QString& msg)
	{
		ShowAnswer (msg);
	}

	void IrcServerHandler::GetUsers (const QString&,
			const QList<std::string>&, const QString& msg)
	{
		ShowAnswer (msg);
	}

	void IrcServerHandler::GetNoUser (const QString&,
			const QList<std::string>&, const QString& msg)
	{
		ShowAnswer (msg);
	}

	void IrcServerHandler::GetTraceLink (const QString&,
			const QList<std::string>& params, const QString&)
	{
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ShowAnswer (message);
	}

	void IrcServerHandler::GetTraceConnecting (const QString&,
			const QList<std::string>& params, const QString&)
	{
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ShowAnswer (message);
	}

	void IrcServerHandler::GetTraceHandshake (const QString&,
			const QList<std::string>& params, const QString&)
	{
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ShowAnswer (message);
	}

	void IrcServerHandler::GetTraceUnknown (const QString&,
			const QList<std::string>& params, const QString&)
	{
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ShowAnswer (message);
	}

	void IrcServerHandler::GetTraceOperator (const QString&,
			const QList<std::string>& params, const QString&)
	{
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ShowAnswer (message);
	}

	void IrcServerHandler::GetTraceUser (const QString&,
			const QList<std::string>& params, const QString&)
	{
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ShowAnswer (message);
	}

	void IrcServerHandler::GetTraceServer (const QString&,
			const QList<std::string>& params, const QString&)
	{
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ShowAnswer (message);
	}

	void IrcServerHandler::GetTraceService (const QString&,
			const QList<std::string>& params, const QString&)
	{
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ShowAnswer (message);
	}

	void IrcServerHandler::GetTraceNewType (const QString&,
			const QList<std::string>& params, const QString&)
	{
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ShowAnswer (message);
	}

	void IrcServerHandler::GetTraceClass (const QString&,
			const QList<std::string>& params, const QString&)
	{
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ShowAnswer (message);
	}

	void IrcServerHandler::GetTraceLog (const QString&,
			const QList<std::string>& params, const QString&)
	{
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ShowAnswer (message);
	}

	void IrcServerHandler::GetTraceEnd (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		QString server = QString::fromUtf8 (params
				.at (params.count () - 1).c_str ());
		ShowAnswer (server + " " + msg);
	}

	void IrcServerHandler::GetStatsLinkInfo (const QString&,
			const QList<std::string>& params, const QString&)
	{
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ShowAnswer (message);
	}

	void IrcServerHandler::GetStatsCommands (const QString&,
			const QList<std::string>& params, const QString&)
	{
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ShowAnswer (message);
	}

	void IrcServerHandler::GetStatsEnd (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		QString letter = QString::fromUtf8 (params
				.at (params.count () - 1).c_str ());
		ShowAnswer (letter + " " + msg);
	}

	void IrcServerHandler::GetStatsUptime (const QString&,
			const QList<std::string>& , const QString& msg)
	{
		ShowAnswer (msg);
	}

	void IrcServerHandler::GetStatsOline (const QString&,
			const QList<std::string>& params, const QString&)
	{
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ShowAnswer (message);
	}

	void IrcServerHandler::GetLuserClient (const QString&,
			const QList<std::string>&, const QString& msg)
	{
		ShowAnswer (msg);
	}

	void IrcServerHandler::GetLuserOp (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		ShowAnswer (QString::fromUtf8 (params.last ().c_str ()) + ":" + msg);
	}

	void IrcServerHandler::GetLuserUnknown (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		ShowAnswer (QString::fromUtf8 (params.last ().c_str ()) + ":" + msg);
	}

	void IrcServerHandler::GetLuserChannels (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		ShowAnswer (QString::fromUtf8 (params.last ().c_str ()) + ":" + msg);
	}

	void IrcServerHandler::GetLuserMe (const QString&,
			const QList<std::string>&, const QString& msg)
	{
		ShowAnswer (msg);
	}

	void IrcServerHandler::GetAdmineMe (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		ShowAnswer (QString::fromUtf8 (params.last ().c_str ()) + ":" + msg);
	}

	void IrcServerHandler::GetAdminLoc1 (const QString&,
			const QList<std::string>&, const QString& msg)
	{
		ShowAnswer (msg);
	}

	void IrcServerHandler::GetAdminLoc2 (const QString&,
			const QList<std::string>&, const QString& msg)
	{
		ShowAnswer (msg);
	}

	void IrcServerHandler::GetAdminEmail (const QString&,
			const QList<std::string>&, const QString& msg)
	{
		ShowAnswer (msg);
	}

	void IrcServerHandler::GetTryAgain (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		QString cmd = QString::fromUtf8 (params.last ().c_str ());
		ShowAnswer (cmd + ":" + msg);
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
				SIGNAL (disconnected ()),
				this,
				SLOT (connectionClosed ()));

		connect (TcpSocket_ptr.get (),
				SIGNAL (error (QAbstractSocket::SocketError)),
				Account_->GetClientConnection ().get (),
				SLOT (handleError (QAbstractSocket::SocketError)));
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
			SendToConsole (IMessage::DIn, str.trimmed ());
			if (!IrcParser_->ParseMessage (str))
				return;

			QString cmd = IrcParser_->GetIrcMessageOptions ().Command_.toLower ();
			if (ErrorHandler_->IsError (cmd.toInt ()))
			{
				ErrorHandler_->HandleError (cmd.toInt (), 
						IrcParser_->GetIrcMessageOptions ().Parameters_,
						IrcParser_->GetIrcMessageOptions ().Message_);
				if (cmd == "433")
				{
					if (OldNickName_.isEmpty ())
						OldNickName_ = NickName_;
					NickCmdError ();
				}
			}
			else if (LastSendId_.isEmpty ())
				IncomingMessage2Server ();
			else if (!LastSendId_.isEmpty () && !Command2Action_.contains (cmd))
				IncomingMessage2Channel (LastSendId_);

			IncomingMessage2Channel ();
		}
	}

	void IrcServerHandler::connectionEstablished ()
	{
		ServerConnectionState_ = Connected;
		emit connected (ServerID_);
		ServerCLEntry_->SetStatus (EntryStatus (SOnline, QString ()));
		IrcParser_->AuthCommand ();
	}

	void IrcServerHandler::connectionClosed ()
	{
		ServerConnectionState_ = NotConnected;
		ServerCLEntry_->SetStatus (EntryStatus (SOffline, QString ()));
		TcpSocket_ptr->close ();
		emit disconnected (ServerID_);
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
