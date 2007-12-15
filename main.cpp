#include <QtGui>
#include <QMutex>
#include <QTime>
#include <QThread>
#include <typeinfo>
#include <fstream>
#include <cstdio>
#include <iostream>
#include <cstring>
#include "mainwindow.h"

QMutex G_DbgMutex;
int G_DbgShiftLevel = 1;

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
	if (std::strstr (message, ": exit"))
		--G_DbgShiftLevel;
	ostr.open (name.c_str (), std::ios::app);
	ostr << "[" << QTime::currentTime ().toString ("HH:mm:ss.zzz").toStdString () << "] [thread ptr " << QThread::currentThread () << "] ";
	for (int i = 1; i < G_DbgShiftLevel; ++i)
		ostr << "    ";
	ostr << message << std::endl;
	ostr.close ();
	if (std::strstr (message, ": enter"))
		++G_DbgShiftLevel;
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
	qCritical () << "======APPLICATION STARTUP======";

	QTranslator transl;
	QString localeName = QLocale::system ().name ();
	transl.load (QString ("leechcraft_") + localeName);
	app.installTranslator (&transl);

	QCoreApplication::setApplicationName ("Leechcraft");
	QCoreApplication::setOrganizationName ("Deviant");

	qRegisterMetaType<QModelIndex> ("QModelIndex");
	qRegisterMetaType<QModelIndex*> ("QModelIndexStar");

	MainWindow::Instance ();
    return app.exec ();
}

