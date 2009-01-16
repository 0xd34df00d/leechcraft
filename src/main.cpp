#include <typeinfo>
#include <iostream>
#include <cstring>
#include <memory>
#include <QtGui/QtGui>
#include <QLocalServer>
#include <QLocalSocket>
#include <plugininterface/proxy.h>
#include <plugininterface/util.h>
#include "mainwindow.h"
#include "debugmessagehandler.h"

bool IsAlreadyRunning ()
{
	QLocalSocket socket;
	socket.connectToServer ("LeechCraft local socket");
	if (socket.waitForConnected ())
		return true;

	// Clear any halted servers
	QLocalServer server;
	server.listen ("LeechCraft local socket");
	return false;
}

bool ParseCommandLine ()
{
	QStringList args = QCoreApplication::arguments ();
	if (args.contains ("-logtoconsole"))
		qInstallMsgHandler (0);
	if (args.contains ("-help"))
	{
		std::cout << "Usage: " << args.at (0).toStdString () << " [arguments]" << std::endl;
		std::cout << std::endl;
		std::cout << "Where arguments could be:" << std::endl;
		std::cout << "-nopupcheck              Do not check whether plugins had changed" << std::endl;
		std::cout << "-logtoconsole            Log all the output to console instead of log files" << std::endl;
		std::cout << "-help                    Show this help message" << std::endl;
		std::cout << std::endl;
		std::cout << "Please remember that installed plugins could have their own command " << std::endl;
		std::cout << "line options." << std::endl;
		return true;
	}
	return false;
}

int main (int argc, char **argv)
{
    int author = 0xd34df00d;

    qInstallMsgHandler (debugMessageHandler);
    QApplication app (argc, argv);

	if (ParseCommandLine ())
		return 0;

	if (IsAlreadyRunning ())
	{
		QMessageBox::critical (0, QObject::tr ("Critical failure"),
				QObject::tr ("LeechCraft is alread running, please close another "
				"instance before starting it."));
		return 1;
	}

	QDir home = QDir::home ();
	if (!home.exists (".leechcraft"))
		if (!home.mkdir (".leechcraft"))
		{
			QMessageBox::critical (0, QObject::tr ("Critical failure"),
					QDir::toNativeSeparators (QObject::tr ("Could not create path %1/.leechcraft")
					.arg (QDir::homePath ())));
			return 2;
		}

    qDebug () << "======APPLICATION STARTUP======";
    qWarning () << "======APPLICATION STARTUP======";

    QCoreApplication::setApplicationName ("Leechcraft");
    QCoreApplication::setOrganizationName ("Deviant");

	std::auto_ptr<QTranslator> translator (LeechCraft::Util::InstallTranslator (""));

    qRegisterMetaType<QModelIndex> ("QModelIndex");
    qRegisterMetaType<QModelIndex*> ("QModelIndexStar");

	LeechCraft::Util::Proxy::Instance ()->SetStrings (QStringList (QObject::tr ("bytes")) <<
			QObject::tr ("KB") <<
			QObject::tr ("MB") <<
			QObject::tr ("GB"));

	std::auto_ptr<LeechCraft::MainWindow> mw (new LeechCraft::MainWindow ());
    return app.exec ();
}

