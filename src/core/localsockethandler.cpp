/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "localsockethandler.h"
#include <cstdlib>
#include <vector>
#include <boost/scoped_array.hpp>
#include <QLocalSocket>
#include <QUrl>
#include "util/util.h"
#include "interfaces/structures.h"
#include "mainwindow.h"
#include "application.h"

namespace LeechCraft
{
	LocalSocketHandler::LocalSocketHandler ()
	: Server_ (new QLocalServer)
	, Window_ (0)
	{
		if (!Server_->listen (Application::GetSocketName ()))
		{
			if (!static_cast<Application*> (qApp)->IsAlreadyRunning ())
			{
				qWarning () << Q_FUNC_INFO
					<< "WTF? We cannot listen() on the local server but aren't running";
				std::exit (Application::EGeneralSocketError);
			}
			else if (!qobject_cast<Application*> (qApp)->GetVarMap ().count ("plugin"))
				std::exit (Application::EAlreadyRunning);
		}
		connect (Server_.get (),
				SIGNAL (newConnection ()),
				this,
				SLOT (handleNewLocalServerConnection ()));
	}

	void LocalSocketHandler::SetMainWindow (MainWindow *parent)
	{
		Window_ = parent;
	}

	void LocalSocketHandler::handleNewLocalServerConnection ()
	{
		if (Window_)
		{
			Window_->show ();
			Window_->activateWindow ();
			Window_->raise ();
		}
		else
			qWarning () << Q_FUNC_INFO
				<< "but Window_ is still NULL";
		std::auto_ptr<QLocalSocket> socket (Server_->nextPendingConnection ());
		// I think 100 msecs would be more than enough for the local
		// connections.
		if (!socket->bytesAvailable ())
			socket->waitForReadyRead (1000);

		QByteArray read = socket->readAll ();
		QDataStream in (read);
		QStringList arguments;
		in >> arguments;
		arguments.removeFirst ();

		std::vector<std::wstring> strings;
		Q_FOREACH (const QString& arg, arguments)
			strings.push_back (arg.toStdWString ());

		boost::program_options::options_description desc;
		boost::program_options::wcommand_line_parser parser (strings);
		auto map = qobject_cast<Application*> (qApp)->Parse (parser, &desc);
		DoLine (map);
	}

	void LocalSocketHandler::pullCommandLine ()
	{
		DoLine (qobject_cast<Application*> (qApp)->GetVarMap ());
	}

	void LocalSocketHandler::DoLine (const boost::program_options::variables_map& map)
	{
		if (!map.count ("entity"))
			return;

		TaskParameters tp;
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

		QString type;
		try
		{
			if (map.find ("type") != map.end ())
				type = QString::fromUtf8 (map ["type"].as<std::string> ().c_str ());
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
		}

		const auto& entities = map ["entity"].as<std::vector<std::wstring>> ();
		Q_FOREACH (const std::wstring& rawEntity, entities)
		{
			QVariant ve;

			const QString entity = QString::fromWCharArray (rawEntity.c_str ());

			if (type == "url")
				ve = QUrl (entity);
			else if (type == "url_encoded")
				ve = QUrl::fromEncoded (entity.toUtf8 ());
			else if (type == "file")
				ve = QUrl::fromLocalFile (entity);
			else
			{
				if (QFile::exists (entity))
					ve = QUrl::fromLocalFile (entity);
				else if (QUrl::fromEncoded (entity.toUtf8 ()).isValid ())
					ve = QUrl::fromEncoded (entity.toUtf8 ());
				else
					ve = entity;
			}

			Entity e = Util::MakeEntity (ve,
					QString (),
					tp);
			emit gotEntity (e);
		}
	}
};

