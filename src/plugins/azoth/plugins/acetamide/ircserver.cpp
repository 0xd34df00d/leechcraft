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
	}

	void IrcServer::JoinChannel (const ChannelOptions& channel)
	{
		IrcParser_->JoinChannel (channel);
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

	void IrcServer::LeaveChannel (const QString& channel)
	{
		IrcParser_->LeaveChannelCommand (channel);
	}

	void IrcServer::authFinished (const QStringList& params)
	{
		State_ = Connected;
		Q_FOREACH (const ChannelOptions& channel, ChannelsQueue_)
		{
			IrcParser_->JoinChannel (channel);
			ActiveChannels_.append (channel);
		}
		ChannelsQueue_.clear ();
	}

	void IrcServer::setTopic (const QStringList& params)
	{
		QString channelKey = QString ("%1@%2")
				.arg (params.at (params.count () - 3), Server_.ServerName_);
		QString serverKey = Server_.ServerName_ + ":" + QString::number (Server_.ServerPort_);
		ServerManager_->SetTopic (serverKey, channelKey, params.at (params.count () - 2));
	}

	void IrcServer::setCLEntries (const QStringList& params)
	{
		QString channelKey = QString ("%1@%2")
				.arg (params.at (params.count () - 3) , Server_.ServerName_);
		QString serverKey = Server_.ServerName_ + ":" + QString::number (Server_.ServerPort_);
		ServerManager_->SetCLEntries (serverKey, channelKey, params.at (params.count () - 2));
	}

	void IrcServer::readMessage (const QStringList& params)
	{
		QString target = params.at (params.count () - 3);
		if (target.startsWith ("#") || 
				target.startsWith ("+") || target.startsWith ("!") || 
				target.startsWith ("&") || target.startsWith ("$"))
		{
			QString channelKey = QString ("%1@%2")
					.arg (target , Server_.ServerName_);
			QString serverKey = Server_.ServerName_ + ":" + QString::number (Server_.ServerPort_);
			ServerManager_->SetMessageIn (serverKey, channelKey, 
					params.at (params.count () - 2), params.last ());
		}
		else
		{
// 			QList<IrcAccount*> list = ServerManager_->GetAccounts (this);
// 			QString channelID = ActiveChannels_.at (0).ChannelName_ + 
// 					"@" + ActiveChannels_.at (0).ServerName_;
// 			Q_FOREACH (IrcAccount *acc, list)
// 			{
// 				acc->GetClientConnection ()->GetChannelCLEntries (channelID);
// 			}
			
		}
	}

	void IrcServer::setNewParticipant (const QStringList& params)
	{
		QString channelKey = QString ("%1@%2")
				.arg (params.at (params.count () - 2).simplified () , Server_.ServerName_);
		QString serverKey = Server_.ServerName_ + ":" + QString::number (Server_.ServerPort_);
		ServerManager_->SetNewParticipant (serverKey, channelKey, params.last ());
	}

	void IrcServer::setUserLeave (const QStringList& params)
	{
		QString channelKey;
		QString leaveMsg;
		if (params.count () > 2)
		{
			channelKey = QString ("%1@%2")
					.arg (params.at (params.count () - 3) , Server_.ServerName_);
			leaveMsg = params.at (params.count () - 2);
		}
		else
		{
			channelKey = QString ("%1@%2")
					.arg (params.first ().simplified (), Server_.ServerName_);
			leaveMsg = QString ();
		}

		QString serverKey = Server_.ServerName_ + ":" + QString::number (Server_.ServerPort_);
		ServerManager_->SetUserLeave (serverKey, channelKey, 
				params.last (), leaveMsg);
	}

	void IrcServer::setUserQuit (const QStringList& params)
	{
		Q_FOREACH (const ChannelOptions& channel, ActiveChannels_)
		{
			QStringList paramsList;
			paramsList << channel.ChannelName_;
			if (params.count () > 1) 
				paramsList << params.first ();
			paramsList << params.last ();

			setUserLeave (paramsList);
		}
	}

};
};
};
