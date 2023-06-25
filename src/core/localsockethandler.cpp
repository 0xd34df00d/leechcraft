/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "localsockethandler.h"
#include <cstdlib>
#include <QDataStream>
#include <QLocalServer>
#include <QLocalSocket>
#include <QUrl>
#include "interfaces/structures.h"
#include "application.h"
#include "entitymanager.h"
#include "clargs.h"

namespace LC
{
	LocalSocketHandler::LocalSocketHandler ()
	: Server_ { std::make_unique<QLocalServer> () }
	, EM_ { std::make_unique<EntityManager> (nullptr, nullptr) }
	{
		StartServer ();

		connect (Server_.get (),
				SIGNAL (newConnection ()),
				this,
				SLOT (handleNewLocalServerConnection ()));
	}

	LocalSocketHandler::~LocalSocketHandler () = default;

	void LocalSocketHandler::handleNewLocalServerConnection ()
	{
		std::unique_ptr<QLocalSocket> socket (Server_->nextPendingConnection ());
		if (!socket->bytesAvailable ())
			socket->waitForReadyRead (2000);

		if (!socket->bytesAvailable ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no data read from the socket";
			return;
		}

		QByteArray read = socket->readAll ();
		QDataStream in (read);
		QStringList arguments;
		QString foreignPath;
		in >> arguments >> foreignPath;

		qDebug () << Q_FUNC_INFO << arguments << foreignPath;

		for (const auto& entity : CL::Parse (arguments, foreignPath).Entities_)
			EntityManager { nullptr, nullptr }.HandleEntity (entity);
	}

	namespace
	{
		bool ResetStale (QLocalServer& server)
		{
			if (server.serverError () != QAbstractSocket::AddressInUseError)
				return false;

			QLocalSocket socket;
			socket.connectToServer (Application::GetSocketName ());
			if (socket.waitForConnected () ||
					socket.error () != QLocalSocket::ConnectionRefusedError)
				return false;

			qDebug () << Q_FUNC_INFO
					<< "clearing stale local server...";
			QLocalServer::removeServer (Application::GetSocketName ());
			return true;
		}
	}

	void LocalSocketHandler::StartServer ()
	{
		if (Server_->listen (Application::GetSocketName ()))
			return;

		qDebug () << Q_FUNC_INFO << "unable to listen:" << Server_->errorString ();

		if (ResetStale (*Server_) &&
				Server_->listen (Application::GetSocketName ()))
			return;

		qDebug () << Q_FUNC_INFO << "unable to listen [2]:" << Server_->errorString ();
		if (!static_cast<Application*> (qApp)->IsAlreadyRunning ())
		{
			qWarning () << Q_FUNC_INFO
					<< "WTF? We cannot listen() on the local server but aren't running";
			std::exit (Application::EGeneralSocketError);
		}
		else if (qobject_cast<Application*> (qApp)->GetParsedArguments ().Plugins_.isEmpty ())
			std::exit (Application::EAlreadyRunning);
	}
}
