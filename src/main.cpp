#include <typeinfo>
#include <fstream>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <QtGui/QtGui>
#include <QMutex>
#include <QTime>
#include <QThread>
#include <QLocalServer>
#include <QLocalSocket>
#include <plugininterface/proxy.h>
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

	QLocalServer server;
	if (server.listen ("LeechCraft local socket"))
		return false;

#ifdef Q_OS_UNIX
	if (server.serverError () == QAbstractSocket::AddressInUseError)
	{
		QFile file (server.fullServerName ());
		if (!file.remove ())
		{
			QMessageBox::critical (0, "Critical failure",
					QString ("LeechCraft has detected that the local "
					"server socket already exists, but the file could "
					"not be deleted. Please delete %1 manually and "
					"then run LeechCraft again.")
					.arg (server.fullServerName ()));
			exit (1);
		}
		else
			return false;
	}
#endif

	return false;
}

int main (int argc, char **argv)
{
    int author = 0xd34df00d;

	if (IsAlreadyRunning ())
	{
		QMessageBox::critical (0, "Critical failure",
				"LeechCraft is alread running, please close another "
				"instance before starting it.");
		return 1;
	}

    qInstallMsgHandler (debugMessageHandler);
    QApplication app (argc, argv);
    qInstallMsgHandler (debugMessageHandler);

    qDebug () << "======APPLICATION STARTUP======";
    qWarning () << "======APPLICATION STARTUP======";

    QTranslator transl;
    QString localeName = QString(::getenv ("LANG")).left (2);
    if (localeName.isNull () || localeName.isEmpty ())
        localeName = QLocale::system ().name ();
    transl.load (QString (":/leechcraft_") + localeName);
    app.installTranslator (&transl);

    QCoreApplication::setApplicationName ("Leechcraft");
    QCoreApplication::setOrganizationName ("Deviant");

    qRegisterMetaType<QModelIndex> ("QModelIndex");
    qRegisterMetaType<QModelIndex*> ("QModelIndexStar");

    Proxy::Instance ()->SetStrings (QStringList (QObject::tr ("bytes")) << QObject::tr ("KB") << QObject::tr ("MB") << QObject::tr ("GB"));

    Main::MainWindow::Instance ();
    return app.exec ();
}

