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

#include "util.h"
#include <stdexcept>
#include <QString>
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QFile>
#include <QDir>
#include <QTemporaryFile>
#include <QSettings>
#include <QtDebug>
#include "proxy.h"

QTranslator* LeechCraft::Util::InstallTranslator (const QString& baseName,
		const QString& prefix,
		const QString& appName)
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName ());
	QString localeName = settings.value ("Language", "system").toString ();

	if (localeName == "system")
	{
		localeName = QString(::getenv ("LANG"));
		if (localeName.isEmpty () || localeName.size () != 5)
			localeName = QLocale::system ().name ();
		localeName = localeName.left (5);
	}

	if (localeName.size () == 2)
		localeName += "_00";

	QString filename = prefix;
	filename.append ("_");
	if (!baseName.isEmpty ())
		filename.append (baseName).append ("_");
	filename.append (localeName);

	QTranslator *transl = new QTranslator;
#ifdef Q_WS_WIN
	if (transl->load (filename, ":/") ||
			transl->load (filename, "translations"))
#else
	if (transl->load (filename, ":/") ||
			transl->load (filename,
				QString ("/usr/local/share/%1/translations").arg (appName)) ||
			transl->load (filename,
				QString ("/usr/share/%1/translations").arg (appName)))
#endif
	{
		qApp->installTranslator (transl);
		return transl;
	}

	qWarning () << Q_FUNC_INFO
		<< "could not load translation file for locale"
		<< localeName
		<< filename;
	return 0;
}

QDir LeechCraft::Util::CreateIfNotExists (const QString& path)
{
	QDir home = QDir::home ();
	home.cd (".leechcraft");
	if (home.exists (path) && !home.cd (path))
	{
		throw std::runtime_error (qPrintable (QObject::tr ("Could not cd into %1")
					.arg (QDir::toNativeSeparators (home.filePath (path)))));
	}
	home = QDir::home ();
	home.cd (".leechcraft");
	if (!home.exists (path) && !home.mkpath (path))
	{
		throw std::runtime_error (qPrintable (QObject::tr ("Could not create %1")
					.arg (QDir::toNativeSeparators (home.filePath (path)))));
	}

	home = QDir::home ();
	home.cd (".leechcraft");
	home.cd (path);
	return home;
}

QString LeechCraft::Util::GetTemporaryName (const QString& pattern)
{
	QTemporaryFile file (QDir::tempPath () + "/" + pattern);
	file.open ();
	QString name = file.fileName ();
	file.close ();
	file.remove ();
	return name;
}

LeechCraft::DownloadEntity LeechCraft::Util::MakeEntity (const QVariant& entity,
		const QString& location,
		LeechCraft::TaskParameters tp,
		const QString& mime)
{
	LeechCraft::DownloadEntity result;
	result.Entity_ = entity;
	result.Location_ = location;
	result.Parameters_ = tp;
	result.Mime_ = mime;
	return result;
}

