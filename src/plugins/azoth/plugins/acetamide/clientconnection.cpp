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

#include "clientconnection.h"
#include <QTextCodec>
#include <plugininterface/util.h>
#include <interfaces/iprotocol.h>
#include <interfaces/iproxyobject.h>
#include "channelclentry.h"
#include "channelhandler.h"
#include "core.h"
#include "ircprotocol.h"
#include "ircserverclentry.h"
#include "ircserverhandler.h"
#include "ircserverconsole.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ClientConnection::ClientConnection (IrcAccount *account)
	: Account_ (account)
	, ProxyObject_ (0)
	{
		QObject *proxyObj = qobject_cast<IrcProtocol*> (account->
				GetParentProtocol ())->GetProxyObject ();
		ProxyObject_ = qobject_cast<IProxyObject*> (proxyObj);
	}

	QObject* ClientConnection::GetCLEntry (const QString& id,
			const QString& nickname) const
	{
		QString idc = id.mid (id.indexOf ('_') + 1,
				id.indexOf ('/') - id.indexOf ('_') - 1);
		if (id.contains ("/Console") && ServerHandlers_.contains (idc))
			return ServerHandlers_ [idc]->GetIrcServerConsole ().get ();
		if (ServerHandlers_.contains (id) && nickname.isEmpty ())
			return ServerHandlers_ [id]->GetCLEntry ();
		else if (!nickname.isEmpty ())
			return ServerHandlers_ [id]->GetParticipantEntry (nickname)
					.get ();
		else
			Q_FOREACH (IrcServerHandler *ish, ServerHandlers_.values ())
				if (ish->IsChannelExists (id))
					return ish->GetChannelHandler (id)->GetCLEntry ();

		return NULL;
	}

	void ClientConnection::Sinchronize ()
	{
	}

	IrcAccount* ClientConnection::GetAccount () const
	{
		return Account_;
	}

	bool ClientConnection::IsServerExists (const QString& key)
	{
		return ServerHandlers_.contains (key);
	}

	void ClientConnection::JoinServer (const ServerOptions& server)
	{
		QString serverId = server.ServerName_ + ":" +
				QString::number (server.ServerPort_);

		IrcServerHandler *ish = new IrcServerHandler (server, Account_);
		ServerHandlers_ [serverId] = ish;
		ish->ConnectToServer ();
	}

	void  ClientConnection::JoinChannel (const ServerOptions& server,
			const ChannelOptions& channel)
	{
		QString serverId = server.ServerName_ + ":" +
				QString::number (server.ServerPort_);
		QString channelId = channel.ChannelName_ + "@" +
				channel.ServerName_;

		if (ServerHandlers_ [serverId]->IsChannelExists (channelId))
		{
			Entity e = Util::MakeNotification ("Azoth",
				tr ("This channel is already joined."),
				PCritical_);
			Core::Instance ().SendEntity (e);
			return;
		}

		if (!ServerHandlers_ [serverId]->JoinChannel (channel))
		{
			Entity e = Util::MakeNotification ("Azoth",
					tr ("Unable to join the channel."),
					PCritical_);
			Core::Instance ().SendEntity (e);
			return;
		}
	}

	IrcServerHandler*
			ClientConnection::GetIrcServerHandler (const QString& id)
	{
		return ServerHandlers_ [id];
	}

	void ClientConnection::ClosePrivateChat (QString serverId,
			const QString& nick)
	{
		ServerHandlers_ [serverId]->ClosePrivateChat (nick);
	}

	void ClientConnection::CloseServer (const QString& serverId)
	{
		if (ServerHandlers_.contains (serverId))
			if (ServerHandlers_ [serverId]->DisconnectFromServer ())
			{
				Account_->
						handleEntryRemoved (ServerHandlers_ [serverId]->
							GetCLEntry ());
				ServerHandlers_.remove (serverId);
				if (!ServerHandlers_.count ())
					Account_->ChangeState (EntryStatus (SOffline,
							QString ()));
			}
	}

	void ClientConnection::DisconnectFromAll ()
	{
		Q_FOREACH (IrcServerHandler *ish, ServerHandlers_.values ())
		{
			Q_FOREACH (ChannelHandler* ich, ish->GetChannelHandlers ())
				ich->LeaveChannel (QString ());
			Q_FOREACH (const QString& nick, ish->GetPrivateChats ())
				ish->ClosePrivateChat (nick);
			if (ish->GetIrcServerConsole ())
				Account_->handleEntryRemoved (ish->GetIrcServerConsole ()
						.get ());
			ish->DisconnectFromServer ();
			ServerHandlers_.remove (ish->GetServerID_ ());
			Account_->handleEntryRemoved (ish->GetCLEntry ());
		}
	}

	void ClientConnection::QuitServer (const QStringList& list)
	{
		IrcServerHandler *ish = ServerHandlers_ [list.last ()];
		Q_FOREACH (ChannelHandler* ich, ish->GetChannelHandlers ())
			ich->LeaveChannel (QString ());
		Q_FOREACH (const QString& nick, ish->GetPrivateChats ())
			ish->ClosePrivateChat (nick);
		if (ish->GetIrcServerConsole ())
			Account_->handleEntryRemoved (ish->GetIrcServerConsole ()
					.get ());
		ish->DisconnectFromServer ();
		ServerHandlers_.remove (ish->GetServerID_ ());
		Account_->handleEntryRemoved (ish->GetCLEntry ());
	}

	void ClientConnection::serverConnected (const QString& serverId)
	{
		if (Account_->GetState ().State_ == SOffline)
			Account_->ChangeState (EntryStatus (SOnline, QString ()));
		emit gotRosterItems (QList<QObject*> () <<
				ServerHandlers_ [serverId]->GetCLEntry ());
	}

	void ClientConnection::handleError (QAbstractSocket::SocketError)
	{
		QTcpSocket *socket = qobject_cast<QTcpSocket*> (sender ());
		if (!socket)
		{
			qWarning () << Q_FUNC_INFO
					<< "is not an object of TcpSocket"
					<< sender ();
			return;
		}

		Entity e = Util::MakeNotification ("Azoth",
				socket->errorString (),
				PCritical_);
		Core::Instance ().SendEntity (e);
		Account_->ChangeState (EntryStatus (SOffline, QString ()));
	}
};
};
};
