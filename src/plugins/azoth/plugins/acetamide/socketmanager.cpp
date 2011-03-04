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

#include "socketmanager.h"
#include <QString>
#include <QTcpSocket>
#include <QtDebug>
#include "ircparser.h"
#include <QDateTime>
#include "ircaccount.h"
#include "ircserver.h"
#include "clientconnection.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	SocketManager::SocketManager (QObject *parent)
	: QObject (parent)
	{
	}
	
	SocketManager::~SocketManager ()
	{
		QMap<QString, QTcpSocket*> server2socket;
		Q_FOREACH (server2socket, Account2Server2Socket_.values ())
			qDeleteAll (server2socket);
	}

	void SocketManager::SendCommand(const QString& cmd, const ServerOptions& server, IrcAccount *account)
	{
		QString serverKey = server.ServerName_ + ":" + QString::number (server.ServerPort_);
		QString accountKey = account->GetAccountID ();
		
		if (!Account2Server2Socket_.contains (account))
			CurrentSocket_ = CreateSocket (account, serverKey);
		else 
			CurrentSocket_ = Account2Server2Socket_ [account] [serverKey];
		
		if (!CurrentSocket_)
			return;
		qDebug () << CurrentSocket_ << cmd;
		SendData (cmd);
	}

	
	bool SocketManager::IsConnected (IrcAccount *account, const QString& key)
	{
		return Account2Server2Socket_ [account].contains (key);
	}

	QTcpSocket* SocketManager::CreateSocket (IrcAccount *account, const QString& serverKey)
	{
		QTcpSocket *socket = new QTcpSocket;
		QStringList paramList = serverKey.split (':');
		if (Connect (socket, paramList.at (0), paramList.at (1)))
		{
			QMap<QString, QTcpSocket*> map;
			map [serverKey] = socket;
			Account2Server2Socket_ [account] = map;
			return socket;
		}
		
		return 0;
	}

	int SocketManager::Connect (QTcpSocket *socket, const QString& host, const QString& port)
	{
		socket->connectToHost (host, port.toInt ());
		
		if (!socket->waitForConnected (30000))
		{
			qWarning () << Q_FUNC_INFO
					<< "can't connect to"
					<< host
					<< "on"
					<< port;
			return 0;
		}
		
		InitSocket (socket);
		QMap<QString, QTcpSocket*> map;
		map [QString ("%1:%2").arg (host, port)] = socket;
		return 1;
	}

	void SocketManager::SendData (const QString& data)
	{
		if (!CurrentSocket_->isWritable ())
		{
			qWarning () << Q_FUNC_INFO
					<< CurrentSocket_->error ()
					<< CurrentSocket_->errorString ();
			return;
		}
			
		if (CurrentSocket_->write (data.toAscii ()) == -1)
		{
			qWarning () << Q_FUNC_INFO
					<< CurrentSocket_->error ()
					<< CurrentSocket_->errorString ();
			return;
		}
	}

	void SocketManager::InitSocket (QTcpSocket *socket)
	{
		connect (socket,
				SIGNAL (connected ()),
				this,
				SLOT (connectionEstablished ()));
		
		connect (socket,
				SIGNAL (readyRead ()),
				this,
				SLOT (readAnswer ()));
	}
	
	void SocketManager::connectionEstablished ()
	{
		QTcpSocket *socket = qobject_cast<QTcpSocket*> (sender ());
		if (!socket)
		{
			qWarning () << Q_FUNC_INFO
					<< "is not an object of QTcpSocket"
					<< sender ();
			return;
		}
		
		qDebug () << "connected to" 
				<< socket->peerName ()
				<< "on"
				<< socket->peerPort ();
	}

	void SocketManager::readAnswer ()
	{
		QTcpSocket *socket = qobject_cast<QTcpSocket*> (sender ());
		if (!socket)
			return;
		
		QString serverKey = socket->peerName () +
				":" +
				QString::number (socket->peerPort ());
		
		QMap <QString, QTcpSocket*> map;
		map [serverKey] = socket;
		IrcAccount *acc = Account2Server2Socket_.key (map); 
		
		while (socket->canReadLine ())
	 		acc->GetClientConnection ()->GetServer (serverKey)->
					GetParser ()->handleServerReply (socket->readLine ());
	}
};
};
};
