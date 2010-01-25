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

#include "application.h"
#include <typeinfo>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <boost/scoped_array.hpp>
#include <QEvent>
#include <QtDebug>
#include <QLocalServer>
#include <QLocalSocket>
#include <QDir>
#include <QIcon>
#include <QMessageBox>
#include <QMetaType>
#include <QModelIndex>
#include <QSessionManager>
#include <QProcess>
#include <QTimer>
#include <plugininterface/util.h>
#include <plugininterface/structuresops.h>
#include "debugmessagehandler.h"
#include "tagsmanager.h"
#include "mainwindow.h"
#include "config.h"

using namespace LeechCraft;

LeechCraft::Application::Application (int& argc, char **argv)
: QApplication (argc, argv)
, CatchExceptions_ (true)
{
	Arguments_ = arguments ();

	// Always show help
	if (Arguments_.contains ("-help"))
	{
		std::cout << "Usage: "
			<< Arguments_.at (0).toStdString ()
			<< " [arguments] [entity]" << std::endl;
		std::cout << std::endl;
		std::cout
			<< "Entity is something that could (possibly) be handled by LeechCraft."
			<< std::endl
			<< "For example, an URL or a torrent file." << std::endl;
		std::cout << "Arguments could be:"<< std::endl;
		std::cout
			<< "-automatic    Don't consider this entity to be added by the user." << std::endl;
		std::cout
			<< "-handle       This entity should be handled, it shouldn't be downloaded."
			<< std::endl;
		std::cout
			<< "-download     This entity should be downloaded but not handled."
			<< std::endl;
		std::cout
			<< "-nolog        Disable custom logger and print everything to the "
			<< std::endl
			<< "              stdout/stderr in a raw form."
			<< std::endl;
		std::cout
			<< "-bt           Print backtraces into logs (makes sense only if "
			<< std::endl
			<< "              compiled with _GNU_SOURCE defined)." << std::endl;
		std::cout
			<< "-clrsckt      Clear stalled socket if previous LeechCraft instance"
			<< std::endl
			<< "              terminated not cleanly and there are stale sockets"
			<< std::endl
			<< "              that LeechCraft is unable to detect as stale (but"
			<< std::endl
			<< "              you are sure they are)."
			<< std::endl;
		std::cout
			<< "-autorestart  Automatically restart application if it's closed (this is"
			<< std::endl
			<< "              done via the Session Manager, so it is not guaranteed to"
			<< std::endl
			<< "              work everywhere, especially on Windows and Mac OS X)."
			<< std::endl;
		std::cout
			<< "-no-app-catch Don't catch exceptions in QApplication::notify"
			<< std::endl;
		std::cout
			<< "-help         Show this help message." << std::endl;
		std::cout << std::endl;
		std::cout
			<< "Installed plugins may have their own command line options."
			<< std::endl;
		std::cout
			<< "There are maybe some other hidden arguments which alter the program's"
			<< std::endl
			<< "behavior in an advanced or experimental way. Hack through the source code"
			<< std::endl
			<< "if you want them."
			<< std::endl;
		std::exit (EHelpRequested);
	}

	if (Arguments_.contains ("-no-app-catch"))
		CatchExceptions_ = false;

	if (Arguments_.contains ("-clrsckt"))
		QLocalServer::removeServer (GetSocketName ());

	if (Arguments_.contains ("-restart"))
	{
		EnterRestartMode ();
		return;
	}
	
	// Sanity checks
	if (IsAlreadyRunning ())
		std::exit (EAlreadyRunning);

	Util::InstallTranslator ("", "qt", "qt4");

	QDir home = QDir::home ();
	if (!home.exists (".leechcraft"))
		if (!home.mkdir (".leechcraft"))
		{
			QMessageBox::critical (0,
					tr ("LeechCraft"),
					QDir::toNativeSeparators (tr ("Could not create path %1/.leechcraft")
						.arg (QDir::homePath ())));
			std::exit (EPaths);
		}

	// Things are sane, prepare
    QCoreApplication::setApplicationName ("Leechcraft");
	QCoreApplication::setApplicationVersion (LEECHCRAFT_VERSION);
    QCoreApplication::setOrganizationName ("Deviant");

	Translator_.reset (LeechCraft::Util::InstallTranslator (""));

    qRegisterMetaType<QModelIndex> ("QModelIndex");
    qRegisterMetaType<QModelIndex*> ("QModelIndexStar");
	qRegisterMetaType<TagsManager::TagsDictionary_t> ("LeechCraft::TagsManager::TagsDictionary_t");
	qRegisterMetaType<LeechCraft::DownloadEntity> ("LeechCraft::DownloadEntity");
	qRegisterMetaType<LeechCraft::Notification> ("LeechCraft::Notification");
	qRegisterMetaTypeStreamOperators<TagsManager::TagsDictionary_t> ("LeechCraft::TagsManager::TagsDictionary_t");
	qRegisterMetaTypeStreamOperators<LeechCraft::DownloadEntity> ("LeechCraft::DownloadEntity");

	ParseCommandLine ();

	setWindowIcon (QIcon (":/resources/images/leechcraft.svg"));
	
	// Say hello to logs
    qDebug () << "======APPLICATION STARTUP======";
    qWarning () << "======APPLICATION STARTUP======";

	// And finally!..
	new LeechCraft::MainWindow ();
}

const QStringList& Application::Arguments () const
{
	return Arguments_;
}

#ifdef Q_WS_WIN
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#endif

QString Application::GetSocketName ()
{
	QString templ = QString ("LeechCraft_local_socket_%1");
#ifdef Q_WS_WIN
	boost::scoped_array<TCHAR> buffer (new TCHAR [0]);
	DWORD size = 0;
	GetUserName (buffer.get (), &size);
	buffer.reset (new TCHAR [size]);
	if (GetUserName (buffer.get (), &size))
		return templ.arg (buffer.get ());
	else
		return templ.arg ("unknown");
#else
	return templ.arg (getuid ());
#endif
}

void Application::InitiateRestart ()
{
	QStringList arguments = Arguments_;
	arguments << "-restart";

	QProcess::startDetached (applicationFilePath (), arguments);

	qApp->quit ();
}

bool Application::notify (QObject *obj, QEvent *event)
{
	if (CatchExceptions_)
	{
		try
		{
			return QApplication::notify (obj, event);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< QString::fromUtf8 (e.what ())
				<< typeid (e).name ()
				<< "for"
				<< obj
				<< event
				<< event->type ();
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
				<< obj
				<< event
				<< event->type ();
		}
		return false;
	}
	else
		return QApplication::notify (obj, event);
}

void Application::commitData (QSessionManager& sm)
{
	if (Arguments_.contains ("-autorestart"))
		sm.setRestartHint (QSessionManager::RestartImmediately);

	sm.release ();
}

void Application::saveState (QSessionManager& sm)
{
	if (Arguments_.contains ("-autorestart"))
		sm.setRestartHint (QSessionManager::RestartImmediately);

	sm.release ();
}

void Application::checkStillRunning ()
{
	if (IsAlreadyRunning ())
		return;

	QStringList arguments;
	if (Arguments_.contains ("-autorestart"))
		arguments << "-autorestart";
	if (Arguments_.contains ("-nolog"))
		arguments << "-nolog";
	if (Arguments_.contains ("-bt"))
		arguments << "-bt";
	if (Arguments_.contains ("-no-app-catch"))
		arguments << "-no-app-catch";

	QProcess::startDetached (applicationFilePath (), arguments);

	quit ();
}

bool Application::IsAlreadyRunning () const
{
	QLocalSocket socket;
	socket.connectToServer (GetSocketName ());
	if (socket.waitForConnected () ||
			socket.state () == QLocalSocket::ConnectedState)
	{
		QDataStream out (&socket);
		out << Arguments_;
		if (socket.waitForBytesWritten ())
			return true;
        if (socket.error() == QLocalSocket::UnknownSocketError)
            return true;
	}
	else
	{
		switch (socket.error ())
		{
			case QLocalSocket::ServerNotFoundError:
			case QLocalSocket::ConnectionRefusedError:
				break;
			default:
				qWarning () << Q_FUNC_INFO
					<< "socket error"
					<< socket.error ();
				return true;
		}
	}

	// Clear any halted servers and their messages
	QLocalServer::removeServer (GetSocketName ());
	return false;
}

void Application::ParseCommandLine ()
{
	if (Arguments_.contains ("-nolog"))
	{
		qInstallMsgHandler (0);
		Arguments_.removeAll ("-nolog");
		Arguments_.removeAll ("-bt");
	}

	if (Arguments_.contains ("-bt"))
	{
		qInstallMsgHandler (DebugHandler::backtraced);
		Arguments_.removeAll ("-bt");
	}
	else
		qInstallMsgHandler (DebugHandler::simple);
}

void Application::EnterRestartMode ()
{
	QTimer *timer = new QTimer;
	connect (timer,
			SIGNAL (timeout ()),
			this,
			SLOT (checkStillRunning ()));
	timer->start (1000);
} 

