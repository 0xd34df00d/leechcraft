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

#include "ircservermanager.h"
#include <boost/bind.hpp>
#include "ircaccount.h"
#include "ircmessage.h"
#include "clientconnection.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcServerManager::IrcServerManager (QObject *parent)
	: QObject (parent)
	{
	}

	void IrcServerManager::JoinChannel (const ServerOptions& server, 
			const ChannelOptions& channel, IrcAccount *account)
	{
		QString serverKey = server.ServerName_ + ":" + QString::number (server.ServerPort_);
		QList<IrcServer_ptr> list = Account2Server_.value (account).values (serverKey);
		if (!list.isEmpty ())
		{
			IrcServer_ptr srv = list.first ();
			switch (srv->GetState ())
			{
			case NotConnected:
				break;
			case InProcess:
				srv->AddChannel2Queue (channel);
				break;
			case Connected:
				srv->JoinChannel (channel);
				break;
			}
		}
		else
		{
			if (Account2Server_ [account].contains (serverKey))
				return;

			IrcServer_ptr ircserver (new IrcServer (server, this));
			Account2Server_ [account] [serverKey] = ircserver;
			ircserver->ConnectToServer ();
			ircserver->AddChannel2Queue (channel);
		}
	}

	void IrcServerManager::SetTopic (const QString& serverKey,
			const QString& channelKey, const QString& topic)
	{
		DoClientConnectionAction (boost::bind (&ClientConnection::setSubject, _1, topic, channelKey),
				serverKey);
	}

	void IrcServerManager::SetCLEntries (const QString& serverKey,
			const QString& channelKey, const QString& clentries)
	{
		DoClientConnectionAction (boost::bind (&ClientConnection::setChannelUseres, _1, clentries, channelKey),
				serverKey);
	}

	void IrcServerManager::SetMessageIn (const QString& serverKey, 
			const QString& channelKey, const QString& message, const QString& nick)
	{
		DoClientConnectionAction (boost::bind (&ClientConnection::handleMessageReceived, _1, message, channelKey, nick),
				serverKey);
	}

	void IrcServerManager::SetMessageOut (const QString& message,
			const ChannelOptions& channel, const ServerOptions& server, IrcAccount *acc)
	{
		QString key = server.ServerName_ + ":" +
				QString::number (server.ServerPort_);
		Account2Server_ [acc] [key]->SendPublicMessage (message, channel);
	}

	void IrcServerManager::SetPrivateMessageOut (IrcAccount *acc, IrcMessage *msg)
	{
		QString key = msg->GetID ();
		if (!Account2Server_ [acc].contains (key))
		{
			qWarning () << Q_FUNC_INFO
					<< "not found server with key"
					<< key;
			return;
		}
		Account2Server_ [acc] [key]->SendPrivateMessage (msg);
	}

	void IrcServerManager::LeaveChannel (const QString& channel, const QString& key, IrcAccount *acc)
	{
		if (!Account2Server_ [acc].contains (key))
		{
			qWarning () << Q_FUNC_INFO
					<< "not found server with key"
					<< key;
			return;
		}

		Account2Server_ [acc] [key]->LeaveChannel (channel);
	}

	void IrcServerManager::SetNewParticipant (const QString& serverKey,
			const QString& channelKey, const QString& nick)
	{
		DoClientConnectionAction (boost::bind (&ClientConnection::SetNewParticipant, _1, channelKey, nick),
				serverKey);
	}

	void IrcServerManager::SetUserLeave (const QString& serverKey,
			const QString& channelKey, const QString& nick, const QString& msg)
	{
		DoClientConnectionAction (boost::bind (&ClientConnection::SetUserLeave, _1, channelKey, nick, msg),
				serverKey);
	}

	bool IrcServerManager::DoServerAction (boost::function<void (IrcServer_ptr)> action, const QString& key)
	{
		QMap<IrcAccount*, QHash<QString, IrcServer_ptr> >::const_iterator iter;

		for (iter = Account2Server_.constBegin (); iter != Account2Server_.constEnd (); ++iter)
			if (ServerExists (iter.key (), key))
			{
				action (iter.value ().value (key));
				return true;
			}

		qWarning () << Q_FUNC_INFO
				<< "not found server with key"
				<< key;

		return false;
	}

	bool IrcServerManager::DoClientConnectionAction (boost::function<void (ClientConnection*)> action, 
			const QString& key)
	{
		QList<IrcAccount*> accList;
		QMap<IrcAccount*, QHash<QString, IrcServer_ptr> >::const_iterator iter;

		for (iter = Account2Server_.constBegin (); iter != Account2Server_.constEnd (); ++iter)
			if (ServerExists (iter.key (), key))
				accList << iter.key ();

		Q_FOREACH (IrcAccount *acc, accList)
		{
			action (acc->GetClientConnection ().get ());
			return true;
		}

		qWarning () << Q_FUNC_INFO
				<< "not found server with key"
				<< key;

		return false;
	}

	IrcAccount* IrcServerManager::GetAccount (IrcServer *server)
	{
		QMap<IrcAccount*, QHash<QString, IrcServer_ptr> >::const_iterator iter;

		for (iter = Account2Server_.constBegin (); iter != Account2Server_.constEnd (); ++iter)
			if (iter.value ().contains (server->GetServerKey ()) &&
					iter.value ().value (server->GetServerKey ())->
							GetNickName () == server->GetNickName ())
				return iter.key ();
		return 0;
	}

	IrcServer_ptr IrcServerManager::GetServer (const QString& key, IrcAccount *acc)
	{
		return Account2Server_ [acc] [key];
	}

	bool IrcServerManager::ServerExists (IrcAccount *acc, const QString& key)
	{
		return Account2Server_ [acc].contains (key);
	}

	void IrcServerManager::changeState (const QString& serverKey, ConnectionState state)
	{
		DoServerAction (boost::bind (&IrcServer::ChangeState, _1, state), serverKey);
	}

	void IrcServerManager::handleAnswer (const QString& serverKey, const QString& answer)
	{
		DoServerAction (boost::bind (&IrcServer::ReadAnswer, _1, answer), serverKey);
	}

	void IrcServerManager::removeServer (const QString& key)
	{
		QMap<IrcAccount*, QHash<QString, IrcServer_ptr> >::iterator iter;

		for (iter = Account2Server_.begin (); iter != Account2Server_.end (); ++iter)
		{
			if (ServerExists (iter.key (), key))
			{
				iter.value ().value (key)->ChangeState (NotConnected);
				iter.value ().remove (key);
				if (!iter.value ().count ())
					iter.key ()->ChangeState (EntryStatus (SOffline, QString ()));
			}
			
		}
		Core::Instance ().GetSocketManager ()->CloseSocket (key);
	}
};
};
};

uint qHash (const LeechCraft::Azoth::Acetamide::IrcServer_ptr& server)
{
	return qHash (server.get ());
}
