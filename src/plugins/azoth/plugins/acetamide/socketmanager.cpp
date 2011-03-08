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
#include "core.h"

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
		qDeleteAll (Server2Socket_);
	}

	void SocketManager::SendCommand (const QString& cmd, const QString& host, int port)
	{
		QString serverKey = host + ":" + QString::number (port);
		
		if (!Server2Socket_.contains (serverKey))
			CurrentSocket_ = CreateSocket (serverKey);
		else 
			CurrentSocket_ = Server2Socket_ [serverKey];

		if (!CurrentSocket_)
			return;

		qDebug () << CurrentSocket_ << cmd;

		SendData (cmd);
	}

	bool SocketManager::IsConnected (const QString& key)
	{
		return Server2Socket_.contains (key);
	}

	QTcpSocket* SocketManager::CreateSocket (const QString& serverKey)
	{
		QTcpSocket *socket = new QTcpSocket;
		QStringList paramList = serverKey.split (':');
		if (Connect (socket, paramList.at (0), paramList.at (1)))
		{
			Server2Socket_ [serverKey] = socket;
			emit changeState (serverKey, InProcess);
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
		
		connect (this,
				SIGNAL (changeState (const QString&, ConnectionState)),
				Core::Instance ().GetServerManager ().get (),
				SLOT (changeState (const QString&, ConnectionState)),
				Qt::UniqueConnection);
		
		connect (this,
				SIGNAL (gotAnswer (const QString&, const QString&)),
				Core::Instance ().GetServerManager ().get (),
				SLOT (handleAnswer (const QString&, const QString&)),
				Qt::UniqueConnection);
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

		qDebug () << "connection established with" 
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
		{
			QString str = socket->readLine ();
			emit gotAnswer (Server2Socket_.key (socket), 
					str);
		}
	}
};
};
};
