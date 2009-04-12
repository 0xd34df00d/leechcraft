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
#include <plugininterface/proxy.h>
#include <plugininterface/util.h>
#include "debugmessagehandler.h"

using namespace LeechCraft;

Application::Application (int& argc, char **argv)
: QApplication (argc, argv)
{
	Arguments_ = arguments ();

	// Always show help
	if (Arguments_.contains ("-help"))
	{
		std::cout << "Usage: "
			<< Arguments_.at (0).toStdString ()
			<< " [arguments]" << std::endl;
		std::cout << std::endl;
		std::cout << "Where arguments could be:"<< std::endl;
		std::cout
			<< "-nolog        Disable custom logger and print everything to the "
			<< std::endl
			<< "              stdout/stderr in a raw form."
			<< std::endl;
		std::cout
			<< "-bt           Print backtraces into logs (makes sense only if "
			<< std::endl
			<< "              compiled with _GNU_SOURCE defined)." << std::endl;
		std::cout << "-help         Show this help message." << std::endl;
		std::cout << std::endl;
		std::cout << "Installed plugins could have their own command line options."
			<< std::endl;
		std::cout << "There are maybe some other hidden arguments which alter the program's"
			<< std::endl
			<< "behavior in an advanced or experimental way. Hack through the source code"
			<< std::endl
			<< "if you want them."
			<< std::endl;
		std::exit (EHelpRequested);
	}

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
		qWarning () << "GLOBALLY" << e.what () << "for" << obj << event << event->type ();
	}
	catch (...)
	{
		qWarning () << "GLOBALLY caught something" << obj << event << event->type ();
	}
	return false;
}

bool Application::IsAlreadyRunning () const
{
	QLocalSocket socket;
	socket.connectToServer (GetSocketName ());
	if (socket.serverName ().size () ||
			socket.waitForConnected ())
	{
		QByteArray toSend;
		{
			QDataStream out (&toSend, QIODevice::WriteOnly);
			out << Arguments_;
		}
		socket.write (toSend);
		socket.disconnectFromServer ();
		socket.waitForDisconnected ();
		return true;
	}

	// Clear any halted servers and their messages
	QLocalServer server;
	server.listen (GetSocketName ());
	QLocalSocket *pc = 0;
	while ((pc = server.nextPendingConnection ())) ;
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

