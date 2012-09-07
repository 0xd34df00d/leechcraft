/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
#include <map>
#include <iomanip>
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
#if not defined (Q_OS_WIN32) && not defined (Q_OS_MAC)

		if (!strcmp (message, "QPixmap::handle(): Pixmap is not an X11 class pixmap"))
			return;

#endif
		static const std::map<QtMsgType, QString> fileName =
		{
			{QtDebugMsg, "debug.log"},
			{QtWarningMsg, "warning.log"},
			{QtCriticalMsg, "critical.log"},
			{QtFatalMsg, "fatal.log"}
		};

		const QString name = QDir::homePath () + "/.leechcraft/" + fileName.at (type);

		G_DbgMutex.lock ();

		std::ofstream ostr;
		ostr.open (QDir::toNativeSeparators (name).toStdString ().c_str (), std::ios::app);
		ostr << "["
			 << QDateTime::currentDateTime ().toString ("dd.MM.yyyy HH:mm:ss.zzz").toStdString ()
			 << "] ["
			 << QThread::currentThread ()
			 << "] ["
			 << std::setfill ('0')
			 << std::setw (3)
			 << Counter++
			 << "] "
			 << message
			 << std::endl;

#ifdef _GNU_SOURCE

		if (type != QtDebugMsg && bt) {
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
