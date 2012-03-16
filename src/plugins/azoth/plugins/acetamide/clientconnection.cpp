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
#include "xmlsettingsmanager.h"

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

	void ClientConnection::Sinchronize ()
	{
	}

	IrcAccount* ClientConnection::GetAccount () const
	{
		return Account_;
	}

	QList<IrcServerHandler*> ClientConnection::GetServerHandlers () const
	{
		return ServerHandlers_.values ();
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

		if (!channel.ChannelName_.isEmpty ())
			ServerHandlers_ [serverId]->JoinChannel (channel);
	}

	void ClientConnection::SetBookmarks (const QList<IrcBookmark>& bookmarks)
	{
		QList<QVariant> res;
		Q_FOREACH (const IrcBookmark& bookmark, bookmarks)
		{
			QByteArray result;
			{
				QDataStream ostr (&result, QIODevice::WriteOnly);
				ostr << bookmark.Name_
						<< bookmark.ServerName_
						<< bookmark.ServerPort_
						<< bookmark.ServerEncoding_
						<< bookmark.ChannelName_
						<< bookmark.ChannelPassword_
						<< bookmark.NickName_
						<< bookmark.SSL_
						<< bookmark.AutoJoin_;
			}

			res << QVariant::fromValue (result);
		}
		XmlSettingsManager::Instance ().setProperty ("Bookmarks",
				QVariant::fromValue (res));
	}

	QList<IrcBookmark> ClientConnection::GetBookmarks () const
	{
		QList<QVariant> list = XmlSettingsManager::Instance ().Property ("Bookmarks",
				QList<QVariant> ()).toList ();

		QList<IrcBookmark> bookmarks;
		Q_FOREACH (const QVariant& variant, list)
		{
			IrcBookmark bookmark;
			QDataStream istr (variant.toByteArray ());
			istr >> bookmark.Name_
					>> bookmark.ServerName_
					>> bookmark.ServerPort_
					>> bookmark.ServerEncoding_
					>> bookmark.ChannelName_
					>> bookmark.ChannelPassword_
					>> bookmark.NickName_
					>> bookmark.SSL_
					>> bookmark.AutoJoin_;

			bookmarks << bookmark;
		}
		return bookmarks;
	}

	IrcServerHandler* ClientConnection::GetIrcServerHandler (const QString& id) const
	{
		return ServerHandlers_ [id];
	}

	void ClientConnection::DisconnectFromAll ()
	{
		Q_FOREACH (auto ish, ServerHandlers_.values ())
			ish->SendQuit ();
	}

	void ClientConnection::QuitServer (const QStringList& list)
	{
		auto ish = ServerHandlers_ [list.last ()];
		ish->DisconnectFromServer ();
	}

	void ClientConnection::SetConsoleEnabled (bool enabled)
	{
		IsConsoleEnabled_ = enabled;
		Q_FOREACH (auto srv, ServerHandlers_.values ())
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

	void ClientConnection::ClosePrivateChat (const QString& serverID, QString nick)
	{
		if (ServerHandlers_.contains (serverID))
			ServerHandlers_ [serverID]->ClosePrivateChat (nick);
	}

	void ClientConnection::FetchVCard (const QString& serverId, const QString& nick)
	{
		if (!ServerHandlers_.contains (serverId))
			return;

		ServerHandlers_ [serverId]->VCardRequest (nick);
	}

	void ClientConnection::SetAway (bool away, const QString& message)
	{
		QString msg = message;
		if (msg.isEmpty ())
			msg = GetStatusStringForState (SAway);

		if (!away)
			msg.clear ();

		QList<IrcServerHandler*> handlers = ServerHandlers_.values ();
		std::for_each (handlers.begin (), handlers.end (),
				[msg] (decltype (handlers.front ()) handler)
				{
					handler->SetAway (msg);
				});
	}

	QString ClientConnection::GetStatusStringForState (State state)
	{
		const QString& statusKey = "DefaultStatus" + QString::number (state);
		return ProxyObject_->GetSettingsManager ()->
				property (statusKey.toUtf8 ()).toString ();
	}

	void ClientConnection::serverConnected (const QString& serverId)
	{
		if (Account_->GetState ().State_ == SOffline)
		{
			Account_->ChangeState (EntryStatus (SOnline, QString ()));
			Account_->SetState (EntryStatus (SOnline, QString ()));
		}
		emit gotRosterItems (QList<QObject*> () <<
				ServerHandlers_ [serverId]->GetCLEntry ());
	}

	void ClientConnection::serverDisconnected (const QString& serverId)
	{
		Account_->handleEntryRemoved (ServerHandlers_ [serverId]->
				GetCLEntry ());
		ServerHandlers_.take (serverId)->deleteLater ();
		if (!ServerHandlers_.count ())
			Account_->SetState (EntryStatus (SOffline,
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
		Account_->SetState (EntryStatus (SOffline, QString ()));
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
