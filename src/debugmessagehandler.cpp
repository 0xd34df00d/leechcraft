#include "debugmessagehandler.h"
#include <fstream>
#include <cstdlib>
#ifdef _GNU_SOURCE
#include <execinfo.h>
#endif
#include <QThread>
#include <QDateTime>
#include <QDir>

QMutex G_DbgMutex;
uint Counter = 0;

void DebugHandler::debugMessageHandler (QtMsgType type, const char *message)
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
	ostr << "["
		<< QDateTime::currentDateTime ().toString ("dd.MM.yyyy HH:mm:ss.zzz").toStdString ()
		<< "] ["
		<< QThread::currentThread ()
		<< "] ["
		<< QString ("%1").arg (Counter++, 3, 10, QChar ('0')).toStdString ()
		<< "] ";
	ostr << message << std::endl;

	if (type != QtDebugMsg && PrintStack_)
	{
		const int maxSize = 100;
		void *array [maxSize];
		size_t size = backtrace (array, maxSize);
		char **strings = backtrace_symbols (array, size);

		ostr << "Backtrace of " << size << " frames:" << std::endl;

		for (int i = 0; i < size; ++i)
			ostr << i << "\t" << strings [i] << std::endl;
		std::free (strings);
	}

	ostr.close ();
	G_DbgMutex.unlock ();
}

