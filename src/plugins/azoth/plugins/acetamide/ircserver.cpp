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
		return IrcParser_->GetNickName ();
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

	void IrcServer::authFinished (const QStringList& params)
	{
		State_ = Connected;
		Q_FOREACH (const ChannelOptions& channel, ChannelsQueue_)
			IrcParser_->JoinChannel (channel);

		ChannelsQueue_.clear ();
	}

	void IrcServer::setTopic (const QStringList& params)
	{
		QString channelKey = QString ("%1@%2")
				.arg (* (params.end () - 3), Server_.ServerName_);
		QString serverKey = Server_.ServerName_ + ":" + QString::number (Server_.ServerPort_);
		ServerManager_->SetTopic (serverKey, channelKey, * (params.end () - 2));
	}

	void IrcServer::setCLEntries (const QStringList& params)
	{
		QString channelKey = QString ("%1@%2")
				.arg (* (params.end () - 3) , Server_.ServerName_);
		QString serverKey = Server_.ServerName_ + ":" + QString::number (Server_.ServerPort_);
		ServerManager_->SetCLEntries (serverKey, channelKey, * (params.end () - 2));
	}

	void IrcServer::readMessage (const QStringList& params)
	{
		qDebug () << params;
		QString channelKey = QString ("%1@%2")
				.arg (* (params.end () - 3) , Server_.ServerName_);
		QString serverKey = Server_.ServerName_ + ":" + QString::number (Server_.ServerPort_);
		ServerManager_->SetMessage (serverKey, channelKey, * (params.end () - 2), params.last ());
	}

};
};
};
