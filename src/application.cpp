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
#include <plugininterface/proxy.h>
#include <plugininterface/util.h>
#include <plugininterface/structuresops.h>
#include "debugmessagehandler.h"
#include "tagsmanager.h"

using namespace LeechCraft;

LeechCraft::Application::Application (int& argc, char **argv)
: QApplication (argc, argv)
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
			<< "-restart      Automatically restart application if it's closed (this is"
			<< std::endl
			<< "              done via the Session Manager, so it is not guaranteed to"
			<< std::endl
			<< "              work everywhere, especially on Windows and Mac OS X)."
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

	if (Arguments_.contains ("-clrsckt"))
		QLocalServer::removeServer (GetSocketName ());
	
	// Sanity checks
	if (IsAlreadyRunning ())
		std::exit (EAlreadyRunning);

	QDir home = QDir::home ();
	if (!home.exists (".leechcraft"))
		if (!home.mkdir (".leechcraft"))
		{
			QMessageBox::critical (0, tr ("Critical failure"),
					QDir::toNativeSeparators (tr ("Could not create path %1/.leechcraft")
					.arg (QDir::homePath ())));
			std::exit (EPaths);
		}

	// Things are sane, prepare
    QCoreApplication::setApplicationName ("Leechcraft");
    QCoreApplication::setOrganizationName ("Deviant");

	Translator_.reset (LeechCraft::Util::InstallTranslator (""));

    qRegisterMetaType<QModelIndex> ("QModelIndex");
    qRegisterMetaType<QModelIndex*> ("QModelIndexStar");
	qRegisterMetaType<TagsManager::TagsDictionary_t> ("LeechCraft::TagsManager::TagsDictionary_t");
	qRegisterMetaType<LeechCraft::DownloadEntity> ("LeechCraft::DownloadEntity");
	qRegisterMetaTypeStreamOperators<TagsManager::TagsDictionary_t> ("LeechCraft::TagsManager::TagsDictionary_t");
	qRegisterMetaTypeStreamOperators<LeechCraft::DownloadEntity> ("LeechCraft::DownloadEntity");

	LeechCraft::Util::Proxy::Instance ()->SetStrings (QStringList (tr ("bytes")) <<
			tr ("KB") <<
			tr ("MB") <<
			tr ("GB"));

	ParseCommandLine ();

	setWindowIcon (QIcon (":/resources/images/mainapp.png"));
	
	// Say hello to logs
    qDebug () << "======APPLICATION STARTUP======";
    qWarning () << "======APPLICATION STARTUP======";
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

bool Application::notify (QObject *obj, QEvent *event)
{
	try
	{
		return QApplication::notify (obj, event);
	}
	catch (const std::exception& e)
	{
		qWarning () << Q_FUNC_INFO << e.what () << "for" << obj << event << event->type ();
	}
	catch (...)
	{
		qWarning () << Q_FUNC_INFO << obj << event << event->type ();
	}
	return false;
}

void Application::commitData (QSessionManager& sm)
{
	if (Arguments_.contains ("-restart"))
		sm.setRestartHint (QSessionManager::RestartImmediately);

	sm.release ();
}

void Application::saveState (QSessionManager& sm)
{
	if (Arguments_.contains ("-restart"))
		sm.setRestartHint (QSessionManager::RestartImmediately);

	sm.release ();
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

