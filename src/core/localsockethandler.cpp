/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "localsockethandler.h"
#include <cstdlib>
#include <vector>
#include <QDataStream>
#include <QLocalServer>
#include <QLocalSocket>
#include <QUrl>
#include <QFile>
#include <QDir>
#include "util/xpc/util.h"
#include "interfaces/structures.h"
#include "application.h"

namespace LC
{
	LocalSocketHandler::LocalSocketHandler ()
	: Server_ (new QLocalServer)
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

		std::vector<std::wstring> strings;
		for (const auto& arg : arguments)
			strings.push_back (arg.toStdWString ());

		boost::program_options::options_description desc;
		boost::program_options::wcommand_line_parser parser (strings);
		auto map = qobject_cast<Application*> (qApp)->Parse (parser, &desc);
		DoLine (map, foreignPath);
	}

	void LocalSocketHandler::pullCommandLine ()
	{
		DoLine (qobject_cast<Application*> (qApp)->GetVarMap (), QDir::currentPath ());
	}

	namespace
	{
		QString GetType (const boost::program_options::variables_map& map)
		{
			const auto pos = map.find ("type");
			if (pos == map.end ())
				return {};

			return QString::fromStdString (pos->second.as<std::string> ());
		}

		QVariantMap GetAdditionalMap (const boost::program_options::variables_map& map)
		{
			QVariantMap addMap;

			const auto pos = map.find ("additional");
			if (pos == map.end ())
				return addMap;

			for (const auto& add : pos->second.as<std::vector<std::string>> ())
			{
				const auto& str = QString::fromStdString (add);
				const auto& name = str.section (':', 0, 0);
				const auto& value = str.section (':', 1);
				if (value.isEmpty ())
				{
					qWarning () << Q_FUNC_INFO
							<< "malformed Additional parameter:"
							<< str;
					continue;
				}

				addMap [name] = value;
			}
			return addMap;
		}

		TaskParameters GetTaskParams (const boost::program_options::variables_map& map)
		{
			TaskParameters tp { FromCommandLine };
			if (map.count ("automatic"))
				tp |= AutoAccept;
			else
				tp |= FromUserInitiated;

			if (map.count ("handle"))
			{
				tp |= OnlyHandle;
				tp |= AutoAccept;
			}

			if (map.count ("download"))
			{
				tp |= OnlyDownload;
				tp |= AutoAccept;
			}

			return tp;
		}

		QUrl ResolveLocalFile (const QString& entity, const QString& curDir)
		{
			if (QDir::isAbsolutePath (entity))
				return QUrl::fromLocalFile (entity);

			if (!QFileInfo { curDir }.isDir ())
				return QUrl::fromLocalFile (entity);

			return QUrl::fromLocalFile (QDir { curDir }.filePath (entity));
		}
	}

	void LocalSocketHandler::DoLine (const boost::program_options::variables_map& map, const QString& curDir)
	{
		if (!map.count ("entity"))
			return;

		const auto& tp = GetTaskParams (map);
		const auto& type = GetType (map);
		const auto& addMap = GetAdditionalMap (map);

		for (const auto& rawEntity : map ["entity"].as<std::vector<std::wstring>> ())
		{
			QVariant ve;

			const auto& entity = QString::fromWCharArray (rawEntity.c_str ());

			if (type == "url")
				ve = QUrl (entity);
			else if (type == "url_encoded")
				ve = QUrl::fromEncoded (entity.toUtf8 ());
			else if (type == "file")
				ve = ResolveLocalFile (entity, curDir);
			else
			{
				if (QFile::exists (entity))
					ve = QUrl::fromLocalFile (entity);
				else if (QUrl::fromEncoded (entity.toUtf8 ()).isValid ())
					ve = QUrl::fromEncoded (entity.toUtf8 ());
				else
					ve = entity;
			}

			auto e = Util::MakeEntity (ve,
					{},
					tp);
			e.Additional_ = addMap;
			qDebug () << e.Entity_ << e.Additional_;
			emit gotEntity (e);
		}
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
		else if (!qobject_cast<Application*> (qApp)->GetVarMap ().count ("plugin"))
			std::exit (Application::EAlreadyRunning);
	}
}
