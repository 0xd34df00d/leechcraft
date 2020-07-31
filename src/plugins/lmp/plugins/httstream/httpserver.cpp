/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "httpserver.h"
#include <QTcpServer>
#include <QTcpSocket>

namespace LC
{
namespace LMP
{
namespace HttStream
{
	HttpServer::HttpServer (QObject *parent)
	: QObject { parent }
	, Server_ { new QTcpServer { this } }
	{
		connect (Server_,
				&QTcpServer::newConnection,
				this,
				&HttpServer::HandleNewConnection);
	}

	void HttpServer::SetAddress (const QString& host, int port)
	{
		if (Server_->serverAddress () == QHostAddress { host } &&
				Server_->serverPort () == port)
			return;

		qDebug () << Q_FUNC_INFO << host << port;

		if (Server_->isListening ())
		{
			qDebug () << "server is already listening, stopping...";
			Server_->close ();
		}

		if (!Server_->listen (QHostAddress { host }, port))
		{
			const auto& msg = Server_->errorString ();
			qWarning () << Q_FUNC_INFO
					<< "cannot listen on"
					<< host
					<< port
					<< msg;
		}
	}

	namespace
	{
		void Write (QTcpSocket *socket, const QList<QByteArray>& strings)
		{
			for (const auto& string : strings)
			{
				socket->write (string);
				socket->write ("\r\n");
			}

			socket->write ("\r\n");
		}
	}

	void HttpServer::HandleSocket (QTcpSocket *socket)
	{
		if (!socket->canReadLine ())
			return;

		disconnect (socket,
				&QTcpSocket::readyRead,
				this,
				nullptr);

		auto line = socket->readLine ();
		line.chop (QString (" HTTP/1.0\r\n").size ());

		auto deleteSocket = [socket]
		{
			connect (socket,
					&QTcpSocket::bytesWritten,
					&QTcpSocket::deleteLater);
		};

		if (line.startsWith ("HEAD "))
		{
			const auto& str = line.mid (5) == "/" ?
					"HTTP/1.0 200 OK" :
					"HTTP/1.0 404 Not Found";
			Write (socket, { str });
			deleteSocket ();
		}
		else if (line.startsWith ("GET "))
		{
			if (line.mid (4) == "/")
			{
				Write (socket,
					{
						"HTTP/1.0 200 OK",
						"Content-Type: audio/ogg",
						"Cache-Control: no-cache",
						"Server: LeechCraft LMP"
					});

				{
					QWriteLocker locker { &MapLock_ };
					Socket2FD_ [socket] = socket->socketDescriptor ();
				}

				emit gotClient (socket->socketDescriptor ());
			}
			else
			{
				Write (socket, { "HTTP/1.0 404 Not Found" });
				deleteSocket ();
			}
		}
		else
		{
			Write (socket, { "HTTP/1.0 400 Bad Request" });
			deleteSocket ();
		}
	}

	void HttpServer::HandleNewConnection ()
	{
		const auto socket = Server_->nextPendingConnection ();
		connect (socket,
				&QTcpSocket::disconnected,
				this,
				[this, socket] { HandleDisconnected (socket); });

		if (socket->canReadLine ())
			HandleSocket (socket);
		else
			connect (socket,
					&QTcpSocket::readyRead,
					this,
					[this, socket] { HandleSocket (socket); });
	}

	void HttpServer::HandleDisconnected (QTcpSocket *sock)
	{
		sock->deleteLater ();

		int sockFd = 0;
		{
			QWriteLocker lock { &MapLock_ };
			if (!Socket2FD_.contains (sock))
				return;

			sockFd = Socket2FD_.take (sock);
		}

		emit clientDisconnected (sockFd);
	}
}
}
}
