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
#include "ircclient.h"
#include <QDateTime>

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	SocketManager::SocketManager (IrcClient *client)
	: Result_ (QString ())
	, Client_ (client)
	{
		connect (this,
				SIGNAL (gotAnswer (const QString&, const QString&)),
				Client_,
				SIGNAL (readyToReadAnswer (const QString&, const QString&)));
	}
	
	SocketManager::~SocketManager ()
	{
		qDeleteAll (Socket2Server_);
	}

	void SocketManager::SendCommand (const ServerOptions& server, 
			const ChannelOptions& channel, const QString& cmd)
	{
		QString key = server.ServerName_ + ":" + 
				QString::number (server.ServerPort_);
		
		if (!Socket2Server_.contains (key))
			CurrentSocket_ = CreateSocket (key);
		else 
			CurrentSocket_ = Socket2Server_ [key];
		
		if (!CurrentSocket_)
			return;
		qDebug () << CurrentSocket_ << cmd;
		SendData (cmd);
	}
	
	void SocketManager::SendCommand(const QString& cmd, const QString& host, int port)
	{
		QString key = host + ":" + QString::number (port);
		
		if (!Socket2Server_.contains (key))
			CurrentSocket_ = CreateSocket (key);
		else 
			CurrentSocket_ = Socket2Server_ [key];
		
		if (!CurrentSocket_)
			return;
		qDebug () << CurrentSocket_ << cmd;
		SendData (cmd);
	}

	
	bool SocketManager::IsConnected (const QString& key)
	{
		return Socket2Server_.contains (key);
	}
	
	QString SocketManager::GetResult () const
	{
		return Result_;
	}

	QTcpSocket* SocketManager::CreateSocket (const QString& key)
	{
		QTcpSocket *socket = new QTcpSocket;
		
		if (Connect (socket, key.split (":").at (0), key.split (":").at (1)))
		{
			Socket2Server_ [key] = socket;
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
	 		emit gotAnswer (socket->readLine (), 
					socket->peerName ());
	}
};
};
};
