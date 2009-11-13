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

namespace
{
	void Write (QtMsgType type, const char *message, bool bt)
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

#ifdef _GNU_SOURCE
		if (type != QtDebugMsg && bt)
		{
			const int maxSize = 100;
			void *array [maxSize];
			size_t size = backtrace (array, maxSize);
			char **strings = backtrace_symbols (array, size);

			ostr << "Backtrace of " << size << " frames:" << std::endl;

			for (size_t i = 0; i < size; ++i)
				ostr << i << "\t" << strings [i] << std::endl;
			std::free (strings);
		}
#endif

		ostr.close ();
		G_DbgMutex.unlock ();
	}
};

void DebugHandler::simple (QtMsgType type, const char *message)
{
	Write (type, message, false);
}

void DebugHandler::backtraced (QtMsgType type, const char *message)
{
	Write (type, message, true);
}

