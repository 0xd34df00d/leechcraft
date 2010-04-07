/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include <fstream>
#include <QCoreApplication>
#include <QMutex>
#include <QDir>
#include <QDateTime>
#include <QThread>
#include <QtDebug>
#include "core.h"

QMutex G_DbgMutex;
uint Counter = 0;

void DebugOut (QtMsgType type, const char *message)
{
	QString name (QDir::homePath ());
	name += ("/.leechcraft/");
	switch (type)
	{
		case QtDebugMsg:
			name += "azoth_debug.log";
			break;
		case QtWarningMsg:
			name += "azoth_warning.log";
			break;
		case QtCriticalMsg:
			name += "azoth_critical.log";
			break;
		case QtFatalMsg:
			name += "azoth_fatal.log";
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

	ostr.close ();
	G_DbgMutex.unlock ();
}

int main (int argc, char **argv)
{
	qInstallMsgHandler (DebugOut);
    qDebug () << "======APPLICATION STARTUP======";
    qWarning () << "======APPLICATION STARTUP======";

	QCoreApplication app (argc, argv);
	LeechCraft::Plugins::Azoth::Server::Core::Instance ().Run ();
	return app.exec ();
}

