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

#include "ircserver.h"
#include <QTextCodec>
#include "ircparser.h"
#include "ircmessage.h"
#include "ircaccount.h"
#include "clientconnection.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcServer::IrcServer (const ServerOptions& server, IrcServerManager *serveramanger)
	: ServerManager_ (serveramanger)
	, Server_ (server)
	, IrcParser_ (new IrcParser (this))
	, State_ (NotConnected)
	, Nickname_ (server.ServerNicknames_.at (0))
	{
		connect (this,
				SIGNAL (gotLeaveAllChannels (const QString&)),
				ServerManager_,
				SLOT (removeServer (const QString&)));
	}

	void IrcServer::JoinChannel (const ChannelOptions& channel)
	{
		IrcParser_->JoinChannel (channel);

		if (!ActiveChannels_.contains (channel))
			ActiveChannels_.append (channel);

		if (ChannelsQueue_.contains (channel))
			ChannelsQueue_.removeAll (channel);
	}

	void IrcServer::ConnectToServer ()
	{
		State_ = NotConnected;
		IrcParser_->AuthCommand (Server_);
	}

	boost::shared_ptr<IrcParser> IrcServer::GetParser () const
	{
		return IrcParser_;
	}

	QString IrcServer::GetHost () const
	{
		return Server_.ServerName_;
	}

	int IrcServer::GetPort () const
	{
		return Server_.ServerPort_;
	}

	QString IrcServer::GetServerKey () const
	{
		return Server_.ServerName_ + ":" + QString::number (Server_.ServerPort_);
	}

	ConnectionState IrcServer::GetState () const
	{
		return State_;
	}

	QString IrcServer::GetNickName () const
	{
		return Server_.ServerNicknames_.at (0);
	}

	QString IrcServer::GetEncoding () const
	{
		return Server_.ServerEncoding_;
	}

	void IrcServer::AddChannel2Queue (const ChannelOptions& channel)
	{
		ChannelsQueue_.append (channel);
	}

	QList<ChannelOptions> IrcServer::GetActiveChannels () const
	{
		return ActiveChannels_;
	}

	void IrcServer::ChangeState (ConnectionState state)
	{
		State_ = state;
	}

	void IrcServer::ReadAnswer (const QString& answer)
	{
		IrcParser_->HandleServerReply (answer);
	}

	void IrcServer::SendPublicMessage (const QString& message, const ChannelOptions& channel)
	{
		IrcParser_->PublicMessageCommand (message, channel);
	}

	void IrcServer::SendPrivateMessage (IrcMessage *msg)
	{
		IrcParser_->PrivateMessageCommand (msg->GetBody (), msg->GetOtherVariant ());
	}

	void IrcServer::LeaveChannel (const QString& channel, IrcAccount *acc)
	{
		IrcParser_->LeaveChannelCommand (channel);
		Q_FOREACH (const ChannelOptions& chan, ActiveChannels_)
			if (chan.ChannelName_ == channel)
			{
				ActiveChannels_.removeOne (chan);
				break;
			}
		if (!ActiveChannels_.count () && !ServerManager_->IsPrivateChatExists (GetServerKey (), acc))
			emit gotLeaveAllChannels (GetServerKey ());
	}

	void IrcServer::QuitConnection (const QString& msg)
	{
		IrcParser_->QuitConnectionCommand (msg);
	}

	QHash<QChar, QChar> IrcServer::GetPrefix () const
	{
		return Prefix_;
	}

	void IrcServer::SetRole (const QString& prefix_string)
	{
		QRegExp rexp ("\\(([a-zA-Z]+)\\)(.+)");
		if (rexp.indexIn (prefix_string) > -1)
		{
			QString keys = rexp.cap (1);
			QString vals = rexp.cap (2);
			for (int i = 0; i < keys.length (); ++i)
				Prefix_ [keys [i]] = vals [i];
		}
	}

	void IrcServer::authFinished (const QString&, const QList<std::string>&, const QString&)
	{
		State_ = Connected;
		Q_FOREACH (const ChannelOptions& channel, ChannelsQueue_)
		{
			IrcParser_->JoinChannel (channel);
			if (!ActiveChannels_.contains (channel))
				ActiveChannels_.append (channel);
		}
		ChannelsQueue_.clear ();
	}

	void IrcServer::setTopic (const QString&, const QList<std::string>& params, const QString& topic)
	{
		QString channelKey = QString ("%1@%2")
				.arg (QString::fromUtf8 (params.last ().c_str ()).toLower (), Server_.ServerName_);
		QString serverKey = Server_.ServerName_ + ":" + QString::number (Server_.ServerPort_);
		ServerManager_->SetTopic (serverKey, channelKey, topic);
	}

	void IrcServer::setCLEntries (const QString&, const QList<std::string>& params, const QString& cllist)
	{
		int count = params.count ();
		QString channelKey = QString ("%1@%2")
				.arg (QString::fromUtf8 (params.last ().c_str ()).toLower () , Server_.ServerName_);
		QString serverKey = Server_.ServerName_ + ":" + QString::number (Server_.ServerPort_);
		ServerManager_->SetCLEntries (serverKey, channelKey, cllist);
	}

	void IrcServer::readMessage (const QString& nick, const QList<std::string>& params, const QString& msg)
	{
		int count = params.count ();
		QString target = QString::fromUtf8 (params.last ().c_str ()).toLower ();
		if (target.startsWith ("#") || 
				target.startsWith ("+") || target.startsWith ("!") || 
				target.startsWith ("&") || target.startsWith ("$"))
		{
			QString channelKey = QString ("%1@%2")
					.arg (target , Server_.ServerName_);
			QString serverKey = Server_.ServerName_ + ":" + QString::number (Server_.ServerPort_);
			ServerManager_->SetMessageIn (serverKey, channelKey, 
					msg, nick);
		}
		else
		{
			if (nick == "frigg")
				return;

			IrcAccount *acc = ServerManager_->GetAccount (this);
			if (!acc)
				return;
			ServerParticipantEntry_ptr entry = acc->GetClientConnection ()->
					GetServerParticipantEntry (GetServerKey (), nick);
			IrcMessage *message = new IrcMessage (IMessage::MTChatMessage,
					IMessage::DIn,
					GetHost () + ":" + QString::number (GetPort ()),
					nick,
					acc->GetClientConnection ().get ());
			QTextCodec *codec = QTextCodec::codecForName (GetEncoding ().toUtf8 ());
			QString mess =  codec->toUnicode (msg.toAscii ());
			message->SetBody (mess);
			message->SetDateTime (QDateTime::currentDateTime ());
			
			entry->SetPrivateChat (true);
			
			entry->HandleMessage (message);
		}
	}

	void IrcServer::setNewParticipant (const QString& nick, const QList<std::string>&, const QString& msg)
	{
		qDebug () << nick;
		QString channelKey = QString ("%1@%2")
				.arg (msg.simplified ().toLower (), Server_.ServerName_);
		QString serverKey = Server_.ServerName_ + ":" + QString::number (Server_.ServerPort_);
		ServerManager_->SetNewParticipant (serverKey, channelKey, nick);
	}

	void IrcServer::setUserLeave (const QString& nick, const QList<std::string>& params, const QString& msg)
	{
		QString channelKey;
		QString leaveMsg = QString ();
		int count = params.count ();
		if (count > 2)
		{
			channelKey = QString ("%1@%2")
					.arg (QString::fromUtf8 (params.last ().c_str ()).toLower () , Server_.ServerName_);
			leaveMsg = msg;
		}
		else
			channelKey = QString ("%1@%2")
					.arg (QString::fromUtf8 (params.last ().c_str ()).toLower (), Server_.ServerName_);

		QString serverKey = Server_.ServerName_ + ":" + QString::number (Server_.ServerPort_);
		ServerManager_->SetUserLeave (serverKey, channelKey, 
				nick, leaveMsg);
	}

	void IrcServer::setUserQuit (const QString& nick, const QList<std::string>& params, const QString& msg)
	{
		Q_FOREACH (const ChannelOptions& channel, ActiveChannels_)
			setUserLeave (nick, 
					QList<std::string> () << channel.ChannelName_.toUtf8 ().constData (), msg);
	}

	void IrcServer::setServerSupport (const QString&, const QList<std::string>&, const QString&)
	{
// 		Q_FOREACH (const QString& param, params)
// 			if (param.toLower ().startsWith ("prefix="))
// 				SetRole (param.split ('=').at (1));
	}

};
};
};
