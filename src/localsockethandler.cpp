/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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
#include "plugininterface/util.h"
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
			else
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

		std::vector<std::string> strings;
		Q_FOREACH (const QString& arg, arguments)
			strings.push_back (arg.toStdString ());

		boost::program_options::options_description desc;
		boost::program_options::command_line_parser parser (strings);
		boost::program_options::variables_map map =
				qobject_cast<Application*> (qApp)->Parse (parser, &desc);
		qDebug () << arguments;
		DoLine (map);
	}

	void LocalSocketHandler::pullCommandLine ()
	{
		DoLine (qobject_cast<Application*> (qApp)->GetVarMap ());
	}

	void LocalSocketHandler::DoLine (const boost::program_options::variables_map& map)
	{
		qDebug () << Q_FUNC_INFO << map.count ("entity") << map.size ();
		qDebug () << qobject_cast<Application*> (qApp)->Arguments ();
		for (boost::program_options::variables_map::const_iterator i = map.begin (); i != map.end (); ++i)
			qDebug () << i->first.c_str ();
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
			type = QString::fromUtf8 (map ["type"].as<std::string> ().c_str ());
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
		}

		std::vector<std::string> entities = map ["entity"].as<std::vector<std::string> > ();
		Q_FOREACH (const std::string& entity, entities)
		{
			QVariant ve;

			if (type == "url")
				ve = QUrl (QString::fromUtf8 (entity.c_str ()));
			else if (type == "url_encoded")
				ve = QUrl::fromEncoded (entity.c_str ());
			else
				ve = QString::fromUtf8 (entity.c_str ());

			Entity e = Util::MakeEntity (ve,
					QString (),
					tp);
			emit gotEntity (e);
		}
	}
};

