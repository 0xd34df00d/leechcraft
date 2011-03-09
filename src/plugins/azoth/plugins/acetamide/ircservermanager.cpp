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
		Q_FOREACH (IrcServer_ptr serv, Account2Server.values (account))
			if (serv->GetHost () == server.ServerName_ && 
					serv->GetPort () == server.ServerPort_)
			{
				qDebug () << "FOUND";
				if (serv->GetState () == Connected)
				{
					qDebug () << "CONNECTED";
					serv->JoinChannel (channel);
				}
				else if (serv->GetState () == InProcess)
				{
					qDebug () << "INPROCESS";
					serv->AddChannel2Queue (channel);
				}
				return;
			}

		IrcServer_ptr ircserver (new IrcServer (server, this));
		ircserver->ConnectToServer ();
		ircserver->AddChannel2Queue (channel);
		Account2Server [account] = ircserver;
	}

	void IrcServerManager::SetTopic (const QString& serverKey,
			const QString& channelKey, const QString& topic)
	{
		Q_FOREACH (IrcServer_ptr serv, Account2Server.values ())
			if (serv->GetServerKey () == serverKey)
				Account2Server.key (serv)->GetClientConnection ()->
						setSubject (topic, channelKey);
	}

	void IrcServerManager::SetCLEntries (const QString& serverKey,
			const QString& channelKey, const QString& clentries)
	{
		Q_FOREACH (IrcServer_ptr serv, Account2Server.values ())
			if (serv->GetServerKey () == serverKey)
				Account2Server.key (serv)->GetClientConnection ()->
						setChannelUseres (clentries, channelKey);
	}

	void IrcServerManager::SetMessage (const QString& serverKey, 
			const QString& channelKey, const QString& message)
	{
		Q_FOREACH (IrcServer_ptr serv, Account2Server.values ())
			if (serv->GetServerKey () == serverKey)
				Q_FOREACH (IrcAccount *acc, Account2Server.keys (serv))
					acc->GetClientConnection ()->
							handleMessageReceived (message, channelKey, serv->GetNickName ());
	}

	void IrcServerManager::changeState (const QString& serverKey, ConnectionState state)
	{
		Q_FOREACH (IrcServer_ptr serv, Account2Server.values ())
			if (serv->GetServerKey () == serverKey)
				serv->ChangeState (state);
	}

	void IrcServerManager::handleAnswer (const QString& serverKey, const QString& answer)
	{
		Q_FOREACH (IrcServer_ptr serv, QSet<IrcServer_ptr>::fromList (Account2Server.values ()))
			if (serv->GetServerKey () == serverKey)
				serv->ReadAnswer (answer);
	}
};
};
};

uint qHash (const LeechCraft::Azoth::Acetamide::IrcServer_ptr& server)
{
	return qHash (server.get ());
}
