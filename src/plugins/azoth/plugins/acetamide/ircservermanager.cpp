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
#include "ircaccount.h"
#include "ircmessage.h"
#include "clientconnection.h"
#include <boost/bind.hpp>

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
		Account2Server_ [acc].remove (key);
		QList<QHash<QString, IrcServer_ptr> > list = Account2Server_.values ();
		QList<QHash<QString, IrcServer_ptr> >::iterator iter = std::find_if (list.begin (), list.end (), 
		boost::bind (&IrcServerManager::ServerExists, this, _1, key));
		if (iter == list.end ())
			Core::Instance ().GetSocketManager ()->CloseSocket (key);

		if (!Account2Server_ [acc].count ())
			acc->ChangeState (EntryStatus (SOffline, QString ()));
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

	QList<IrcAccount*> IrcServerManager::GetAccounts (IrcServer *server) const
	{
// 		QList<IrcAccount*> accList;
// 		Q_FOREACH (IrcServer_ptr serv, Account2Server.values ())
// 			if (serv->GetServerKey () == server->GetServerKey ())
// 				accList << Account2Server.key (serv);
// 		return accList;
	}

	void IrcServerManager::RemoveServer (const QString& key)
	{
		QList<QHash<QString, IrcServer_ptr> > list = Account2Server_.values ();
		QList<QHash<QString, IrcServer_ptr> >::iterator iter = std::find_if (list.begin (), list.end (), 
				boost::bind (&IrcServerManager::ServerExists, this, _1, key));
		while (iter != list.end ())
		{
			(*iter).remove (key);
			iter = std::find_if (++iter, list.end (), 
				boost::bind (&IrcServerManager::ServerExists, this, _1, key));
		}
	}

	bool IrcServerManager::ServerExists (QHash<QString, IrcServer_ptr> hash,
			const QString& key)
	{
		return hash.contains (key);
	}
	
	bool IrcServerManager::DoServerAction (boost::function<void (IrcServer_ptr)> action, const QString& key)
	{
		QList<QHash<QString, IrcServer_ptr> > list = Account2Server_.values ();
		QList<QHash<QString, IrcServer_ptr> >::iterator iter = std::find_if (list.begin (), list.end (), 
				boost::bind (&IrcServerManager::ServerExists, this, _1, key));
		if (iter != list.end () )
		{
			action ((*iter).value (key));
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
		QList<QHash<QString, IrcServer_ptr> > list = Account2Server_.values ();
		QList<IrcAccount*> accList;
		QList<QHash<QString, IrcServer_ptr> >::iterator iter;
		iter = std::find_if (list.begin (), list.end (), 
				boost::bind (&IrcServerManager::ServerExists, this, _1, key));

		while (iter != list.end ())
		{
			accList << Account2Server_.key (*iter);
			iter = std::find_if (++iter, list.end (), 
				boost::bind (&IrcServerManager::ServerExists, this, _1, key));
		}
 
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

	void IrcServerManager::changeState (const QString& serverKey, ConnectionState state)
	{
		DoServerAction (boost::bind (&IrcServer::ChangeState, _1, state), serverKey);
	}

	void IrcServerManager::handleAnswer (const QString& serverKey, const QString& answer)
	{
		DoServerAction (boost::bind (&IrcServer::ReadAnswer, _1, answer), serverKey);
	}
};
};
};

uint qHash (const LeechCraft::Azoth::Acetamide::IrcServer_ptr& server)
{
	return qHash (server.get ());
}
