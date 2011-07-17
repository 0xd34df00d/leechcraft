/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
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
#include <util/util.h>
#include <interfaces/iprotocol.h>
#include <interfaces/iproxyobject.h>
#include "channelclentry.h"
#include "channelhandler.h"
#include "core.h"
#include "ircprotocol.h"
#include "ircserverclentry.h"
#include "ircserverhandler.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ClientConnection::ClientConnection (IrcAccount *account)
	: Account_ (account)
	, ProxyObject_ (0)
	, IsConsoleEnabled_ (false)
	{
		QObject *proxyObj = qobject_cast<IrcProtocol*> (account->
				GetParentProtocol ())->GetProxyObject ();
		ProxyObject_ = qobject_cast<IProxyObject*> (proxyObj);
	}

	QObject* ClientConnection::GetCLEntry (const QString& id,
			const QString& nickname) const
	{
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
		
		ish->SetConsoleEnabled (IsConsoleEnabled_);
		if (IsConsoleEnabled_)
			connect (ish,
					SIGNAL (sendMessageToConsole (IMessage::Direction, const QString&)),
					this,
					SLOT (handleLog (IMessage::Direction, const QString&)),
					Qt::UniqueConnection);
		else
			disconnect (ish,
					SIGNAL (sendMessageToConsole (IMessage::Direction, const QString&)),
					this,
					SLOT (handleLog (IMessage::Direction, const QString&)));
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
			ServerHandlers_ [serverId]->DisconnectFromServer ();
	}

	void ClientConnection::DisconnectFromAll ()
	{
		Q_FOREACH (IrcServerHandler *ish, ServerHandlers_.values ())
		{
			ish->LeaveAllChannel ();
			ish->CloseAllPrivateChats ();
			ish->DisconnectFromServer ();
			ServerHandlers_.remove (ish->GetServerID_ ());
			Account_->handleEntryRemoved (ish->GetCLEntry ());
		}
	}

	void ClientConnection::QuitServer (const QStringList& list)
	{
		IrcServerHandler *ish = ServerHandlers_ [list.last ()];
		ish->LeaveAllChannel ();
		ish->CloseAllPrivateChats ();
		ish->DisconnectFromServer ();
		ServerHandlers_.remove (ish->GetServerID_ ());
		Account_->handleEntryRemoved (ish->GetCLEntry ());
	}

	void ClientConnection::SetConsoleEnabled (bool enabled)
	{
		IsConsoleEnabled_ = enabled;
		Q_FOREACH (IrcServerHandler *srv, ServerHandlers_.values ())
		{
			srv->SetConsoleEnabled (enabled);
			if (enabled)
				connect (srv,
						SIGNAL (sendMessageToConsole (IMessage::Direction, const QString&)),
						this,
						SLOT (handleLog (IMessage::Direction, const QString&)),
						Qt::UniqueConnection);
			else
				disconnect (srv,
						SIGNAL (sendMessageToConsole (IMessage::Direction, const QString&)),
						this,
						SLOT (handleLog (IMessage::Direction, const QString&)));
		}
	}

	void ClientConnection::serverConnected (const QString& serverId)
	{
		if (Account_->GetState ().State_ == SOffline)
			Account_->ChangeState (EntryStatus (SOnline, QString ()));
		emit gotRosterItems (QList<QObject*> () <<
				ServerHandlers_ [serverId]->GetCLEntry ());
	}

	void ClientConnection::serverDisconnected (const QString& serverId)
	{
		Account_->handleEntryRemoved (ServerHandlers_ [serverId]->
				GetCLEntry ());
		ServerHandlers_.remove (serverId);
		if (!ServerHandlers_.count ())
			Account_->ChangeState (EntryStatus (SOffline,
					QString ()));
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

	void ClientConnection::handleLog (IMessage::Direction type, const QString& msg)
	{
		switch (type)
		{
		case IMessage::DOut:
			emit gotConsoleLog (msg.toUtf8 (), IHaveConsole::PDOut);
			break;
		case IMessage::DIn:
			emit gotConsoleLog (msg.toUtf8 (), IHaveConsole::PDIn);
			break;
		default:
			break;
		}
	}

};
};
};
