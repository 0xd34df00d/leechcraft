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
#include <interfaces/isyncable.h>
#include <plugininterface/util.h>
#include <plugininterface/structuresops.h>
#include "debugmessagehandler.h"
#include "tagsmanager.h"
#include "mainwindow.h"
#include "config.h"

using namespace LeechCraft;
namespace bpo = boost::program_options;

LeechCraft::Application::Application (int& argc, char **argv)
: QApplication (argc, argv)
, CatchExceptions_ (true)
{
	Arguments_ = arguments ();
	bpo::options_description desc ("Allowed options");
	bpo::command_line_parser parser (argc, argv);
	VarMap_ = Parse (parser, &desc);

	if (VarMap_.count ("help"))
	{
		std::cout << "LeechCraft " << LEECHCRAFT_VERSION << " (http://leechcraft.org)" << std::endl;
		std::cout << std::endl;
		std::cout << desc << std::endl;
		std::exit (EHelpRequested);
	}

	if (VarMap_.count ("version"))
	{
		std::cout << "LeechCraft " << LEECHCRAFT_VERSION << " (http://leechcraft.org)" << std::endl;
		std::exit (EVersionRequested);
	}

	if (VarMap_.count ("no-app-catch"))
		CatchExceptions_ = false;

	if (VarMap_.count ("clrsckt"))
		QLocalServer::removeServer (GetSocketName ());

	if (VarMap_.count ("restart"))
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
					"LeechCraft",
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
	qRegisterMetaType<LeechCraft::Entity> ("LeechCraft::Entity");
	qRegisterMetaType<LeechCraft::Entity> ("Entity");
	qRegisterMetaType<LeechCraft::Sync::ChainID_t> ("LeechCraft::Sync::ChainID_t");
	qRegisterMetaType<LeechCraft::Sync::ChainID_t> ("Sync::ChainID_t");
	qRegisterMetaType<LeechCraft::Sync::ChainID_t> ("ChainID_t");
	qRegisterMetaTypeStreamOperators<TagsManager::TagsDictionary_t> ("LeechCraft::TagsManager::TagsDictionary_t");
	qRegisterMetaTypeStreamOperators<LeechCraft::Entity> ("LeechCraft::Entity");

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

bpo::variables_map Application::Parse (bpo::command_line_parser& parser,
		bpo::options_description *desc) const
{
	bpo::variables_map vm;
	bpo::options_description invisible ("Invisible options");
	invisible.add_options ()
			("entity,E", bpo::value<std::vector<std::string> > (), "the entity to handle");

	desc->add_options ()
			("help", "show the help message")
			("version,v", "print LC version")
			("download,D", "only choose downloaders for the entity: it should be downloaded but not handled")
			("handle,H", "only choose handlers for the entity: it should be handled but not downloaded")
			("type,T", bpo::value<std::string> (), "the type of the entity: url, url_encoded, file (for file paths) and such")
			("automatic", "the entity is a result of some automatic stuff, not user's actions")
			("nolog", "disable custom file logger and print everything to stdout/stderr")
			("bt", "print backtraces for warning messages into warning.log")
			("clrsckt", "clear stalled socket, use if you believe previous LC instance has terminated but failed to close its local socket properly")
			("no-app-catch", "disable exceptions catch-all in QApplication::notify(), useful for debugging purposes")
			("autorestart", "automatically restart LC if it's closed (not guaranteed to work everywhere, especially on Windows and Mac OS X)")
			("restart", "restart the LC");
	bpo::positional_options_description pdesc;
	pdesc.add ("entity", -1);

	bpo::options_description all;
	all.add (*desc);
	all.add (invisible);

	bpo::store (parser
			.options (all)
			.positional (pdesc)
			.allow_unregistered ()
			.run (), vm);
	bpo::notify (vm);

	return vm;
}

const bpo::variables_map& Application::GetVarMap () const
{
	return VarMap_;
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

	QProcess::startDetached (applicationFilePath (), Arguments_);

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
	if (VarMap_.count ("nolog"))
		qInstallMsgHandler (0);
	else if (VarMap_.count ("bt"))
		qInstallMsgHandler (DebugHandler::backtraced);
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

