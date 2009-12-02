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

		DoLine (arguments);
	}

	void LocalSocketHandler::pullCommandLine ()
	{
		QStringList arguments = qobject_cast<Application*> (qApp)->Arguments ();
		DoLine (arguments);
	}

	void LocalSocketHandler::DoLine (const QStringList& arguments)
	{
		if (!(arguments.size () > 1 &&
				!arguments.last ().startsWith ('-')))
			return;

		TaskParameters tp;
		if (!arguments.contains ("-automatic"))
			tp |= FromUserInitiated;
		else
			tp |= AutoAccept;
		if (arguments.contains ("-handle"))
		{
			tp |= OnlyHandle;
			tp |= AutoAccept;
		}
		if (arguments.contains ("-download"))
		{
			tp |= OnlyDownload;
			tp |= AutoAccept;
		}

		QVariant entity;
		int typePos = arguments.indexOf ("-type");
		if (typePos >= 0)
		{
			if (typePos + 1 < arguments.size ())
			{
				QString type = arguments.at (typePos + 1);
				if (type == "url")
					entity = QUrl (arguments.last ());
				else if (type == "url_encoded")
					entity = QUrl::fromEncoded (arguments.last ().toUtf8 ());
				else
					entity = arguments.last ();
			}
			else
			{
				qWarning () << Q_FUNC_INFO
					<< "illegal '-type' option position for"
					<< arguments;
				entity = arguments.last ();
			}
		}
		else
			entity = arguments.last ();

		DownloadEntity e = Util::MakeEntity (entity,
				QString (),
				tp);
		emit gotEntity (e);
	}
};

