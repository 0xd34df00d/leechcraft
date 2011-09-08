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
#include "ircserversocket.h"
#include "usercommandmanager.h"
#include "serverresponcemanager.h"
#include "rplisupportparser.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcServerHandler::IrcServerHandler (const ServerOptions& server,
			IrcAccount *account)
	: Account_ (account)
	, ErrorHandler_ (new IrcErrorHandler (this))
	, IrcParser_ (0)
	, ServerCLEntry_ (new IrcServerCLEntry (this, account))
	, ServerConnectionState_ (NotConnected)
	, IsConsoleEnabled_ (false)
	, IsInviteDialogActive_ (false)
	, IsLongMessageInProcess_ (false)
	, ServerID_ (server.ServerName_ + ":" +
			QString::number (server.ServerPort_))
	, NickName_ (server.ServerNickName_)
	, ServerOptions_ (server)
	{
		IrcParser_ = new IrcParser (this);
		CmdManager_ = new UserCommandManager (this);
		ServerResponceManager_ = new ServerResponceManager (this);
		RplISupportParser_ = new RplISupportParser (this);
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

	QString IrcServerHandler::GetServerID_ () const
	{
		return ServerID_;
	}

	ServerOptions IrcServerHandler::GetServerOptions () const
	{
		return ServerOptions_;
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

	QStringList IrcServerHandler::GetPrivateChats () const
	{
		QStringList result;
		Q_FOREACH (ServerParticipantEntry_ptr spe, Nick2Entry_.values ())
			if (spe->IsPrivateChat ())
				result << spe->GetEntryName ();
		return result;
	}

	ChannelHandler* IrcServerHandler::GetChannelHandler (const QString& id)
	{
		return ChannelHandlers_.contains (id) ?
				ChannelHandlers_ [id] :
				0;
	}

	QList<ServerParticipantEntry_ptr> IrcServerHandler::GetParticipantsOnChannel (const QString& channel)
	{
		QList<ServerParticipantEntry_ptr> result;
		Q_FOREACH (ServerParticipantEntry_ptr spe, Nick2Entry_.values ())
			if (spe->GetChannels ().contains (channel))
				result << spe;
		return result;
	}

	QList<ChannelHandler*> IrcServerHandler::GetChannelHandlers () const
	{
		return ChannelHandlers_.values ();
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

	bool IrcServerHandler::IsChannelExists (const QString& channelID)
	{
		return ChannelHandlers_.contains (channelID);
	}

	bool IrcServerHandler::IsParticipantExists (const QString& nick)
	{
		return Nick2Entry_.contains (nick);
	}

	void IrcServerHandler::SetLongMessageState (bool state)
	{
		IsLongMessageInProcess_ = state;
	}

	bool IrcServerHandler::IsLongMessageInProcess () const
	{
		return IsLongMessageInProcess_;
	}

	void IrcServerHandler::SetNickName (const QString& nick)
	{
		NickName_ = nick;
	}

	void IrcServerHandler::Add2ChannelsQueue (const ChannelOptions& ch)
	{
		if (!ChannelsQueue_.contains (ch) && !ch.ChannelName_.isEmpty ())
			ChannelsQueue_ << ch;
	}

	void IrcServerHandler::JoinChannel (const ChannelOptions& channel)
	{
		QString id = QString (channel.ChannelName_ + "@" +
				channel.ServerName_).toLower ();

		if (ServerConnectionState_ == Connected)
		{
			if (!ChannelHandlers_.contains (id))
				IrcParser_->JoinCommand (channel.ChannelName_ + " " +
						channel.ChannelPassword_);
		}
		else
			Add2ChannelsQueue (channel);
	}

	bool IrcServerHandler::JoinedChannel (const ChannelOptions& channel)
	{
		QString id = QString (channel.ChannelName_ + "@" +
				channel.ServerName_).toLower ();

		if (ServerConnectionState_ == Connected)
		{
			if (!ChannelHandlers_.contains (id))
			{
				ChannelHandler *ch = new ChannelHandler (this, channel);
				ChannelHandlers_ [id] = ch;

				ChannelCLEntry *ichEntry = ch->GetCLEntry ();
				if (!ichEntry)
					return false;
				Account_->handleGotRosterItems (QList<QObject*> () <<
						ichEntry);

				IrcParser_->ChanModeCommand (QStringList () << channel.ChannelName_);
			}
		}
		else
			Add2ChannelsQueue (channel);

		return true;
	}

	void IrcServerHandler::JoinChannelByCmd (const QStringList& cmd)
	{
		if (cmd.isEmpty ())
			return;

		IrcParser_->JoinCommand (cmd.join (" "));
	}

	void IrcServerHandler::JoinParticipant (const QString& nick, 
			const QString& msg)
	{
		QString channelID = (msg + "@" + ServerOptions_.ServerName_).toLower ();

		if (IsChannelExists (channelID))
			ChannelHandlers_ [channelID]->SetChannelUser (nick);
	}

	void IrcServerHandler::CloseChannel (const QString& channel)
	{
		QString channelID = (channel + "@" + ServerOptions_.ServerName_).toLower ();
		if (IsChannelExists (channelID))
			ChannelHandlers_ [channelID]->CloseChannel ();
	}

	void IrcServerHandler::LeaveParticipant (const QString& nick,
			const QString& channel, const QString& msg)
	{
		QString channelID = (channel + "@" + ServerOptions_.ServerName_).toLower ();
		if (IsChannelExists (channelID))
			ChannelHandlers_ [channelID]->LeaveParticipant (nick, msg);
	}

	void IrcServerHandler::QuitServer ()
	{
		Account_->GetClientConnection ()->QuitServer (QStringList () << ServerID_);
	}

	void IrcServerHandler::QuitParticipant (const QString& nick, const QString& msg)
	{
		if (IsParticipantExists (nick))
			Q_FOREACH (const QString& channel, Nick2Entry_ [nick]->GetChannels ())
			{
				QString channelID = channel + "@" + ServerOptions_.ServerName_;
				if (IsChannelExists (channelID))
					ChannelHandlers_ [channelID]->LeaveParticipant (nick,
							msg);
			}
	}

	void IrcServerHandler::SendMessage (const QStringList& cmd)
	{
		if (cmd.isEmpty ())
			return;

		const QString target = cmd.first ();
		const QStringList msg = cmd.mid (1);
		const QString channelID = target + "@" + ServerOptions_.ServerName_;

		if (IsChannelExists (channelID))
			ChannelHandlers_ [channelID]->SendPublicMessage (msg.join (" "));
		else
			IrcParser_->PrivMsgCommand (cmd);
	}

	void IrcServerHandler::IncomingMessage (const QString& nick, 
			const QString& target, const QString& msg)
	{
		const QString channelID = target + "@" + ServerOptions_.ServerName_;
		if (IsChannelExists (channelID))
			ChannelHandlers_ [channelID]->HandleIncomingMessage (nick, msg);
		else
		{
			ServerParticipantEntry_ptr entry = GetParticipantEntry (nick);
			if (!entry)
				return;
			IrcMessage *message = new IrcMessage (IMessage::MTChatMessage,
					IMessage::DIn,
					ServerID_,
					nick,
					Account_->GetClientConnection ().get ());
			message->SetBody (msg);
			message->SetDateTime (QDateTime::currentDateTime ());
			entry->SetStatus (EntryStatus (SOnline, QString ()));
			entry->SetPrivateChat (true);
			entry->HandleMessage (message);
		}
	}

	void IrcServerHandler::IncomingNoticeMessage (const QString& nick, const QString& msg)
	{
		ShowAnswer (msg);
		QList<NickServIdentify> list = Core::Instance ()
				.GetNickServIdentifyWithMainParams (ServerOptions_.ServerName_,
						GetNickName (),
						nick);
		if (list.isEmpty ())
			return;

		Q_FOREACH (const NickServIdentify& nsi, list)
		{
			QRegExp authRegExp (nsi.AuthString_,
					Qt::CaseInsensitive,
					QRegExp::Wildcard);
			if (authRegExp.indexIn (msg) == -1)
				continue;

			SendMessage2Server (nsi.AuthMessage_.split (' '));
			return;
		}
	}

	void IrcServerHandler::ChangeNickname (const QString& nick, 
			const QString& msg)
	{
		if (!IsParticipantExists (nick))
		{
			qWarning () << Q_FUNC_INFO
					<< "there is no such nick"
					<< nick;
			return;
		}

		Q_FOREACH (const QString& channel, Nick2Entry_ [nick]->GetChannels ())
		{
			const QString id = (channel + "@" + ServerOptions_.ServerName_).toLower ();
			const QString mess = tr ("%1 changed nickname to %2").arg (nick, msg);

			if (IsChannelExists (id))
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

	void IrcServerHandler::GetBanList (const QString& channel)
	{
		IrcParser_->ChanModeCommand (QStringList () << channel << "b");
	}

	void IrcServerHandler::GetExceptList (const QString& channel)
	{
		IrcParser_->ChanModeCommand (QStringList () << channel << "e");
	}

	void IrcServerHandler::GetInviteList (const QString& channel)
	{
		IrcParser_->ChanModeCommand (QStringList () << channel << "I");
	}

	void IrcServerHandler::AddBanListItem (const QString& channel, QString mask)
	{
		IrcParser_->ChanModeCommand (QStringList () << channel << "+b" << mask);
	}

	void IrcServerHandler::RemoveBanListItem (const QString& channel, QString mask)
	{
		IrcParser_->ChanModeCommand (QStringList () << channel << "-b" << mask);
	}

	void IrcServerHandler::AddExceptListItem (const QString& channel, QString mask)
	{
		IrcParser_->ChanModeCommand (QStringList () << channel << "+e" << mask);
	}

	void IrcServerHandler::RemoveExceptListItem (const QString& channel, QString mask)
	{
		IrcParser_->ChanModeCommand (QStringList () << channel << "-e" << mask);
	}

	void IrcServerHandler::AddInviteListItem (const QString& channel, QString mask)
	{
		IrcParser_->ChanModeCommand (QStringList () << channel << "+I" << mask);
	}

	void IrcServerHandler::RemoveInviteListItem (const QString& channel, QString mask)
	{
		IrcParser_->ChanModeCommand (QStringList () << channel << "-I" << mask);
	}

	void IrcServerHandler::SetNewChannelModes(const QString& channel, const ChannelModes& modes)
	{
		const QString channelId = (channel + "@" + ServerOptions_.ServerName_).toLower ();
		if (!IsChannelExists (channelId))
			return;

		IrcParser_->ChanModeCommand (QStringList () << channel
				<< (modes.BlockOutsideMessageMode_ ? "+n" : "-n"));
		if (modes.ChannelKey_.first)
			IrcParser_->ChanModeCommand (QStringList () << channel
					<< "+k" << modes.ChannelKey_.second);
		else
			IrcParser_->ChanModeCommand (QStringList () << channel << "-k");
		IrcParser_->ChanModeCommand (QStringList () << channel
				<< (modes.InviteMode_ ? "+i" : "-i"));
		IrcParser_->ChanModeCommand (QStringList () << channel
				<< (modes.ModerateMode_ ? "+m" : "-m"));
		IrcParser_->ChanModeCommand (QStringList () << channel
				<< (modes.OnlyOpChangeTopicMode_ ? "+t" : "-t"));
		IrcParser_->ChanModeCommand (QStringList () << channel
				<< (modes.PrivateMode_ ? "+p" : "-p"));
		IrcParser_->ChanModeCommand (QStringList () << channel
				<< (modes.ReOpMode_ ? "+r" : "-r"));
		IrcParser_->ChanModeCommand (QStringList () << channel
				<< (modes.SecretMode_ ? "+s" : "-s"));
		if (modes.UserLimit_.first)
			IrcParser_->ChanModeCommand (QStringList () << channel
					<< "+l" << QString::number(modes.UserLimit_.second));
		else
			IrcParser_->ChanModeCommand (QStringList () << channel << "-l");
	}

	void IrcServerHandler::PongMessage (const QString& msg)
	{
		IrcParser_->PongCommand (QStringList () << msg);
	}

	void IrcServerHandler::GotTopic (const QString& channel, 
			const QString& message)
	{
		QString channelId = (channel + "@" + ServerOptions_.ServerName_).toLower ();

		if (IsChannelExists (channelId))
			ChannelHandlers_ [channelId]->SetMUCSubject (message);
		else
			ShowAnswer (message);
	}

	void IrcServerHandler::KickUserFromChannel (const QString& nick,
			const QString& channel, const QString& target, 
			const QString& msg)
	{
		QString channelID = (channel + "@" + ServerOptions_.ServerName_).toLower ();

		if (IsChannelExists (channelID))
			ChannelHandlers_ [channelID]->KickParticipant (target, msg, nick);
	}

	void IrcServerHandler::GotInvitation (const QString& nick, 
			const QString& msg)
	{
		if (IsInviteDialogActive_)
			InviteChannelsDialog_->AddInvitation (msg, nick);
		else
		{
			std::auto_ptr<InviteChannelsDialog> dic (new InviteChannelsDialog (msg, nick));
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

	void IrcServerHandler::ShowAnswer (const QString& msg)
	{
		if (!LastSendId_.isEmpty ())
			ChannelHandlers_ [LastSendId_]->ShowServiceMessage (msg,
							IMessage::MTEventMessage,
							IMessage::MSTOther);
		else
			ServerCLEntry_->HandleMessage (CreateMessage (IMessage::MTEventMessage,
					ServerID_, 
					msg));
	}

	void IrcServerHandler::CTCPReply (const QString& nick,
			const QString& cmd, const QString& mess)
	{
		Q_FOREACH (ChannelHandler *ich, ChannelHandlers_.values ())
			ich->ShowServiceMessage (mess,
					IMessage::MTEventMessage,
					IMessage::MSTOther);

		IrcParser_->CTCPReply (QStringList () << nick << cmd);
	}

	void IrcServerHandler::CTCPRequestResult (const QString& msg)
	{
		Q_FOREACH (ChannelHandler *ich, ChannelHandlers_.values ())
			ich->ShowServiceMessage (msg,
					IMessage::MTEventMessage,
					IMessage::MSTOther);
	}

	void IrcServerHandler::GotNames (const QString& channel, 
			const QStringList& participants)
	{
		const QString channelID = (channel + "@" + ServerOptions_.ServerName_).toLower ();
		if (IsChannelExists (channelID) && 
				!ChannelHandlers_ [channelID]->IsRosterReceived ())
			Q_FOREACH (const QString& nick, participants)
				ChannelHandlers_ [channelID]->SetChannelUser (nick);
	}

	void IrcServerHandler::GotEndOfNames (const QString& channel)
	{
		const QString channelID = (channel + "@" + ServerOptions_.ServerName_).toLower ();
		if (IsChannelExists (channelID) && 
				!ChannelHandlers_ [channelID]->IsRosterReceived ())
			ChannelHandlers_ [channelID]->SetRosterReceived (true);
	}

	void IrcServerHandler::ShowUserHost (const QString& nick, 
			const QString& host)
	{
		ShowAnswer (nick + tr (" is a ") + host);
	}

	void IrcServerHandler::ShowIsUserOnServer (const QString& nick)
	{
		ShowAnswer (nick + tr (" is on server"));
	}

	void IrcServerHandler::ShowWhoIsReply (const QString& msg)
	{
		if (!IsLongMessageInProcess ())
			ShowAnswer (tr ("Begin of WHOIS reply:"));
		ShowAnswer (msg);
	}

	void IrcServerHandler::ShowWhoWasReply (const QString& msg)
	{
		if (!IsLongMessageInProcess ())
			ShowAnswer (tr ("Begin of WHOWAS reply:"));
		ShowAnswer (msg);
	}

	void IrcServerHandler::ShowWhoReply (const QString& msg)
	{
		if (!IsLongMessageInProcess ())
			ShowAnswer (tr ("Begin of WHO reply:"));
		ShowAnswer (msg);
	}

	void IrcServerHandler::ShowLinksReply (const QString& msg)
	{
		if (!IsLongMessageInProcess ())
			ShowAnswer (tr ("Begin of LINKS reply:"));
		ShowAnswer (msg);
	}

	void IrcServerHandler::ShowInfoReply (const QString& msg)
	{
		if (!IsLongMessageInProcess ())
			ShowAnswer (tr ("Begin of INFO reply:"));
		ShowAnswer (msg);
	}

	void IrcServerHandler::ShowMotdReply (const QString& msg)
	{
		if (!IsLongMessageInProcess ())
			ShowAnswer (tr ("Begin of MOTD reply:"));
		ShowAnswer (msg);
	}

	void IrcServerHandler::ShowUsersReply (const QString& msg)
	{
		if (!IsLongMessageInProcess ())
			ShowAnswer (tr ("Begin of USERS reply:"));
		ShowAnswer (msg);
	}

	void IrcServerHandler::ShowTraceReply (const QString& msg)
	{
		if (!IsLongMessageInProcess ())
			ShowAnswer (tr ("Begin of TRACE reply:"));
		ShowAnswer (msg);
	}

	void IrcServerHandler::ShowStatsReply (const QString& msg)
	{
		if (!IsLongMessageInProcess ())
			ShowAnswer (tr ("Begin of STATS reply:"));
		ShowAnswer (msg);
	}

	void IrcServerHandler::ShowBanList (const QString& channel, 
			const QString& mask, const QString& nick, const QDateTime& time)
	{
		const QString channelId = (channel + "@" + ServerOptions_.ServerName_).toLower ();
		if (!IsChannelExists (channelId))
			return;

		ChannelHandlers_ [channelId]->SetBanListItem (mask, nick, time);
	}

	void IrcServerHandler::ShowBanListEnd (const QString& msg)
	{
		ShowAnswer (msg);
	}

	void IrcServerHandler::ShowExceptList (const QString& channel, 
			const QString& mask, const QString& nick, const QDateTime& time)
	{
		const QString channelId = (channel + "@" + ServerOptions_.ServerName_).toLower ();
		if (!IsChannelExists (channelId))
			return;

		ChannelHandlers_ [channelId]->SetExceptListItem (mask, nick, time);
	}

	void IrcServerHandler::ShowExceptListEnd (const QString& msg)
	{
		ShowAnswer (msg);
	}

	void IrcServerHandler::ShowInviteList (const QString& channel, 
			const QString& mask, const QString& nick, const QDateTime& time)
	{
		const QString channelId = (channel + "@" + ServerOptions_.ServerName_).toLower ();
		if (!IsChannelExists (channelId))
			return;
		
		ChannelHandlers_ [channelId]->SetInviteListItem (mask, nick, time);
	}

	void IrcServerHandler::ShowInviteListEnd (const QString& msg)
	{
		ShowAnswer (msg);
	}

	void IrcServerHandler::SendPublicMessage (const QString& msg,
			const QString& channelId)
	{
		LastSendId_ = channelId;
		Q_FOREACH (const QString& str, msg.split ('\n'))
			IrcParser_->PrivMsgCommand (QStringList ()
					<< channelId.left (channelId.indexOf ('@'))
					<< str);
	}

	void IrcServerHandler::SendPrivateMessage (IrcMessage* msg)
	{
		LastSendId_ = msg->GetOtherVariant ();
		Q_FOREACH (const QString& str, msg->GetBody ().split ('\n'))
			IrcParser_->PrivMsgCommand (QStringList ()
					<< msg->GetOtherVariant ()
					<< str);

		ServerParticipantEntry_ptr entry = GetParticipantEntry (msg->GetOtherVariant ());
		entry->HandleMessage (msg);
	}

	void IrcServerHandler::SendMessage2Server (const QStringList& list)
	{
		QString msg = list.join (" ");
		if (!CmdManager_->VerifyMessage (msg, QString ()))
		{
			if (msg.startsWith ('/'))
				IrcParser_->RawCommand (msg.mid (1).split (' '));
			else
				IrcParser_->RawCommand (list);
		}
		ShowAnswer (msg);
	}

	void IrcServerHandler::ParseMessageForCommand (const QString& msg,
			const QString& channelID)
	{
		LastSendId_ = channelID;
		if (!CmdManager_->VerifyMessage (msg, channelID.left (channelID.indexOf ('@'))))
			IrcParser_->RawCommand (msg.mid (1).split (' '));
	}

	void IrcServerHandler::LeaveChannel (const QString& channel,
			const QString& msg)
	{
		IrcParser_->PartCommand (QStringList () << channel << msg);
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

	void IrcServerHandler::ConnectToServer ()
	{
		if (ServerConnectionState_ != NotConnected)
			return;

		Socket_ = new IrcServerSocket (this);
		Socket_->ConnectToHost (ServerOptions_.ServerName_,
				ServerOptions_.ServerPort_);
		ServerConnectionState_ = InProgress;
	}

	void IrcServerHandler::DisconnectFromServer ()
	{
		LeaveAllChannel ();
		Q_FOREACH (ChannelHandler *ch, ChannelHandlers_.values ())
			ch->CloseChannel ();
		CloseAllPrivateChats ();
		if (ServerConnectionState_ != NotConnected)
			Socket_->DisconnectFromHost ();
	}

	void IrcServerHandler::SendCommand (const QString& cmd)
	{
		SendToConsole (IMessage::DOut, cmd.trimmed ());
		Socket_->Send (cmd);
	}

	void IrcServerHandler::SendToConsole (IMessage::Direction dir,
			const QString& message)
	{
		if (!IsConsoleEnabled_)
			return;

		if (dir == IMessage::DIn)
			emit sendMessageToConsole (dir, message);
		else
			emit sendMessageToConsole (dir, message);
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

	ServerParticipantEntry_ptr IrcServerHandler::GetParticipantEntry (const QString& nick)
	{
		if (IsParticipantExists (nick))
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
			ch->Leave (msg);
	}

	void IrcServerHandler::CloseAllPrivateChats ()
	{
		Q_FOREACH (ServerParticipantEntry_ptr spe, Nick2Entry_.values ())
			if (spe->IsPrivateChat ())
				spe->closePrivateChat (true);
	}

	void IrcServerHandler::SetLastSendID (const QString& str)
	{
		LastSendId_ = str;
	}

	void IrcServerHandler::ReadReply (const QByteArray& msg)
	{
		SendToConsole (IMessage::DIn, msg.trimmed ());
		if (!IrcParser_->ParseMessage (msg))
			return;
		const QString cmd = IrcParser_->GetIrcMessageOptions ().Command_.toLower ();
		if (ErrorHandler_->IsError (cmd.toInt ()))
		{
			ErrorHandler_->HandleError (cmd.toInt (),
					IrcParser_->GetIrcMessageOptions ().Parameters_,
					IrcParser_->GetIrcMessageOptions ().Message_);
			if (cmd == "433")
			{
				if (OldNickName_.isEmpty ())
					OldNickName_ = NickName_;
				else if (!Account_->GetNickNames ().contains (OldNickName_))
					OldNickName_ = Account_->GetNickNames ().first ();
				NickCmdError ();
			}
		}
		else 
			ServerResponceManager_->DoAction (cmd, 
					IrcParser_->GetIrcMessageOptions ().Nick_,
					IrcParser_->GetIrcMessageOptions ().Parameters_,
					IrcParser_->GetIrcMessageOptions ().Message_);
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
			IrcParser_->JoinCommand (co.ChannelName_ + " " + co.ChannelPassword_);
			ChannelsQueue_.removeAll (co);
		}
	}

	void IrcServerHandler::SayCommand (const QStringList& params)
	{
		if (params.isEmpty ())
			return;
		const QString channel = params.first ();
		SendPublicMessage (QStringList (params.mid (1)).join (" "),
				(channel + "@" + ServerOptions_.ServerName_).toLower ());
	}

	void IrcServerHandler::ParseChanMode (const QString& channel, 
			const QString& mode, const QString& value)
	{
		if (mode.isEmpty ())
			return;

		const QString channelID = (channel + "@" + ServerOptions_.ServerName_).toLower ();
		if (!ChannelHandlers_.contains (channelID))
			return;

		bool action = false;
		if (mode [0] == '+')
			action = true;

		for (int i = 1; i < mode.length (); ++i)
		{
			switch (mode [i].toAscii ())
			{
			case 'o':
				if (!value.isEmpty () && IsParticipantExists (value))
				{
					if (action)
						Nick2Entry_ [value]->AddRole (channel, Operator);
					else
						Nick2Entry_ [value]->RemoveRole (channel, Operator);
				}
				break;
			case 'v':
				if (!value.isEmpty () && IsParticipantExists (value))
				{
					if (action)
						Nick2Entry_ [value]->AddRole (channel, Voiced);
					else
						Nick2Entry_ [value]->RemoveRole (channel, Voiced);
				}
				break;
			case 'a':
				// may be it is nessesary
				break;
			case 'i':
					ChannelHandlers_ [channelID]->SetInviteMode (action);
				break;
			case 'm':
					ChannelHandlers_ [channelID]->SetModerateMode (action);
				break;
			case 'n':
					ChannelHandlers_ [channelID]->SetBlockOutsideMessagesMode (action);
				break;
			case 'q':
				// may be it is nessesary
				break;
			case 'p':
					ChannelHandlers_ [channelID]->SetPrivateMode (action);
				break;
			case 'r':
					ChannelHandlers_ [channelID]->SetServerReOpMode (action);
				break;
			case 's':
					ChannelHandlers_ [channelID]->SetSecretMode (action);
				break;
			case 't':
					ChannelHandlers_ [channelID]->SetOnlyOpTopicChangeMode (action);
				break;
			case 'l':
					ChannelHandlers_ [channelID]->
						SetUserLimit (action, value.toInt ());
				break;
			case 'k':
					ChannelHandlers_ [channelID]->SetChannelKey (action, value);
				break;
			case 'b':
				ShowAnswer (value + tr (" added to your ban list."));
				break;
			case 'e':
				ShowAnswer (value + tr (" added to your except list."));
				break;
			case 'I':
				ShowAnswer (value + tr (" added to your invite list."));
				break;
			}
		}
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
		if (RplISupportParser_->ParseISupportReply (msg))
			ISupport_ = RplISupportParser_->GetISupportMap ();
	}

	QMap<QString, QString> IrcServerHandler::GetISupport () const
	{
		return ISupport_;
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
		Socket_->Close ();
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
