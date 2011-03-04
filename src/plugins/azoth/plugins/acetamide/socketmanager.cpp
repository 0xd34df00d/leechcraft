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
		qDeleteAll (Account2Server2Socket_);
	}

	void SocketManager::SendCommand(const QString& cmd, const ServerOptions& server, IrcAccount *account)
	{
		QString serverKey = server.ServerName_ + ":" + QString::number (server.ServerPort_);
		QString accountKey = account->GetAccountID ();
		
		if (!Account2Server2Socket_.contains (accountKey))
			CurrentSocket_ = CreateSocket (accountKey, serverKey);
		else 
			CurrentSocket_ = Account2Server2Socket_ [accountKey] [serverKey];
		
		if (!CurrentSocket_)
			return;
		qDebug () << CurrentSocket_ << cmd;
		SendData (cmd);
	}

	
	bool SocketManager::IsConnected (const QString& key)
	{
		return Account2Server2Socket_.contains (key);
	}

	QTcpSocket* SocketManager::CreateSocket (const QString& accountKey, const QString& serverKey)
	{
		QTcpSocket *socket = new QTcpSocket;
		
		if (Connect (socket, serverKey.split (":").at (0), serverKey.split (":").at (1)))
		{
			QMap<QString, QTcpSocket*> map;
			map [serverKey] = socket;
			Account2Server2Socket_ [accountKey] = map;
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
		Init (socket);
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
		qDebug () << data.toAscii ();
	}

	void SocketManager::Init (QTcpSocket *socket)
	{
		connect (socket,
				SIGNAL (connected ()),
				this,
				SLOT (connectionEstablished ()));
		
		connect (socket,
				SIGNAL (readyRead ()),
				this,
				SLOT (readAnswer ()));
		
// 		connect (this,
// 				SIGNAL (gotAnswer (const QString&)),
// 				Parser_,
// 				SIGNAL (readyToReadAnswer (const QString&)));
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
		while (socket->canReadLine ())
	 		emit gotAnswer (socket->readLine ());
	}
};
};
};
