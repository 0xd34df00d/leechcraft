#include <QtGui/QtGui>
#include <QMutex>
#include <QTime>
#include <QThread>
#include <typeinfo>
#include <fstream>
#include <cstdio>
#include <iostream>
#include <cstring>
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
    ostr << "[" << QTime::currentTime ().toString ("HH:mm:ss.zzz").toStdString () << "] [thread ptr " << QThread::currentThread () << "] ";
    ostr << message << std::endl;
    ostr.close ();
    G_DbgMutex.unlock ();
}

int main (int argc, char **argv)
{
    int author = 0xd34df00d;

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

