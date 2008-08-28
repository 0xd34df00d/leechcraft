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
    std::string name;
    switch (type)
    {
        case QtDebugMsg:
            name = "debug.log";
            break;
        case QtWarningMsg:
            name = "warning.log";
            break;
        case QtCriticalMsg:
            name = "critical.log";
            break;
        case QtFatalMsg:
            name = "fatal.log";
            break;
    }
    std::ofstream ostr;
    G_DbgMutex.lock ();
    ostr.open (name.c_str (), std::ios::app);
    ostr << "[" << QTime::currentTime ().toString ("HH:mm:ss.zzz").toStdString () << "] [thr " << QThread::currentThread () << "] ";
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
		QMessageBox::critical (0, "Critical failure",
				"LeechCraft is alread running, please close another "
				"instance before starting it.");
		return 1;
	}

    qDebug () << "======APPLICATION STARTUP======";
    qWarning () << "======APPLICATION STARTUP======";

    QCoreApplication::setApplicationName ("Leechcraft");
    QCoreApplication::setOrganizationName ("Deviant");

	LeechCraft::Util::InstallTranslator ("");

    qRegisterMetaType<QModelIndex> ("QModelIndex");
    qRegisterMetaType<QModelIndex*> ("QModelIndexStar");

    Proxy::Instance ()->SetStrings (QStringList (QObject::tr ("bytes")) <<
			QObject::tr ("KB") <<
			QObject::tr ("MB") <<
			QObject::tr ("GB"));

	std::auto_ptr<Main::MainWindow> mw (new Main::MainWindow ());
    return app.exec ();
}

