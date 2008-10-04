#include <typeinfo>
#include <fstream>
#include <iostream>
#include <cstring>
#include <memory>
#include <QtGui/QtGui>
#include <QMutex>
#include <QTime>
#include <QThread>
#include <QLocalServer>
#include <QLocalSocket>
#include <plugininterface/proxy.h>
#include <plugininterface/util.h>
#include "mainwindow.h"

QMutex G_DbgMutex;

void debugMessageHandler (QtMsgType type, const char *message)
{
	QString name (QDir::homePath ());
	name += ("/.leechcraft/");
    switch (type)
    {
        case QtDebugMsg:
            name += "debug.log";
            break;
        case QtWarningMsg:
            name += "warning.log";
            break;
        case QtCriticalMsg:
            name += "critical.log";
            break;
        case QtFatalMsg:
            name += "fatal.log";
            break;
    }
    std::ofstream ostr;
    G_DbgMutex.lock ();
    ostr.open (QDir::toNativeSeparators (name).toStdString ().c_str (), std::ios::app);
    ostr << "[" << QDateTime::currentDateTime ().toString ("dd.MM.yyyy HH:mm:ss.zzz").toStdString ()
	<< "] [thr " << QThread::currentThread () << "] ";
    ostr << message << std::endl;
    ostr.close ();
    G_DbgMutex.unlock ();
}

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
	if (args.contains ("-logToConsole"))
		qInstallMsgHandler (0);
	if (args.contains ("-help"))
	{
		std::cout << "Usage: leechcraft [arguments]" << std::endl << std::endl;
		std::cout << "Where arguments could be:" << std::endl;
		std::cout << "-logToConsole    Logs all output to console instead of log files" << std::endl;
		std::cout << "-help            Show this help message and exit" << std::endl;
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

    Proxy::Instance ()->SetStrings (QStringList (QObject::tr ("bytes")) <<
			QObject::tr ("KB") <<
			QObject::tr ("MB") <<
			QObject::tr ("GB"));

	std::auto_ptr<Main::MainWindow> mw (new Main::MainWindow ());
    return app.exec ();
}

