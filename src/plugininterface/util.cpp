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
#include <QTime>
#include <QSettings>
#include <QTextCodec>
#include <QUrl>
#include <QtDebug>

QString LeechCraft::Util::GetUserText (const DownloadEntity& p)
{
	QString string = QObject::tr ("Too long to show");
	if (p.Additional_.contains ("UserVisibleName") &&
			p.Additional_ ["UserVisibleName"].canConvert<QString> ())
		string = p.Additional_ ["UserVisibleName"].toString ();
	else if (p.Entity_.canConvert<QByteArray> ())
	{
		QByteArray entity = p.Entity_.toByteArray ();
		if (entity.size () < 100)
			string = QTextCodec::codecForName ("UTF-8")->toUnicode (entity);
	}
	else if (p.Entity_.canConvert<QUrl> ())
	{
		string = p.Entity_.toUrl ().toString ();
		if (string.size () > 100)
			string = string.left (97) + "...";
	}
	else
		string = QObject::tr ("Binary entity");

	if (!p.Mime_.isEmpty ())
		string += QObject::tr ("<br /><br />of type <code>%1</code>").arg (p.Mime_);

	if (!p.Additional_ ["SourceURL"].toUrl ().isEmpty ())
	{
		QString urlStr = p.Additional_ ["SourceURL"].toUrl ().toString ();
		if (urlStr.size () > 63)
			urlStr = urlStr.left (60) + "...";
		string += QObject::tr ("<br />from %1")
			.arg (urlStr);
	}

	return string;
}

QString LeechCraft::Util::MakePrettySize (qint64 sourcesize)
{
	int strNum = 0;
	long double size = sourcesize;
	if (size >= 1024)
	{
		strNum = 1;
		size /= 1024;
	}
	if (size >= 1024)
	{
		strNum = 2;
		size /= 1024;
	}
	if (size >= 1024)
	{
		strNum = 3;
		size /= 1024;
	}

	switch (strNum)
	{
		case 0:
			return QString::number (size, 'f', 1) + QObject::tr (" b");
		case 1:
			return QString::number (size, 'f', 1) + QObject::tr (" KiB");
		case 2:
			return QString::number (size, 'f', 1) + QObject::tr (" MiB");
		case 3:
			return QString::number (size, 'f', 1) + QObject::tr (" GiB");
		default:
			return "unknown";
	}
}

QString LeechCraft::Util::MakeTimeFromLong (ulong time)
{
	int d = time / 86400;
	time -= d * 86400;
	QString result;
	if (d)
		result += QObject::tr ("%n day(s), ", "", d);
	result += QTime (0, 0, 0).addSecs (time).toString ();
	return result;
}

QTranslator* LeechCraft::Util::InstallTranslator (const QString& baseName,
		const QString& prefix,
		const QString& appName)
{
	QString localeName = GetLocaleName ();
	QString filename = prefix;
	filename.append ("_");
	if (!baseName.isEmpty ())
		filename.append (baseName).append ("_");
	filename.append (localeName);

	QTranslator *transl = new QTranslator;
#ifdef Q_WS_WIN
	if (transl->load (filename, ":/") ||
			transl->load (filename,
					QCoreApplication::applicationDirPath () + "/translations"))
#elif Q_WS_MAC
	if (transl->load (filename, ":/") ||
			transl->load (filename,
					QCoreApplication::applicationDirPath () + "../Resources/translations"))
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
	delete transl;

	qWarning () << Q_FUNC_INFO
		<< "could not load translation file for locale"
		<< localeName
		<< filename;
	return 0;
}

QString LeechCraft::Util::GetLocaleName ()
{
	QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName ());
	QString localeName = settings.value ("Language", "system").toString ();

	if (localeName == "system")
	{
		localeName = QString (::getenv ("LANG")).left (5);
		if (localeName.isEmpty () || localeName.size () != 5)
			localeName = QLocale::system ().name ();
		localeName = localeName.left (5);
	}

	if (localeName.size () == 2)
	{
		QLocale::Language lang = QLocale (localeName).language ();
		QList<QLocale::Country> cs = QLocale::countriesForLanguage (lang);
		if (cs.isEmpty ())
			localeName += "_00";
		else
			localeName = QLocale (lang, cs.at (0)).name ();
	}

	return localeName;
}

QString LeechCraft::Util::GetLanguage ()
{
	return GetLocaleName ().left (2);
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
	DownloadEntity result;
	result.Entity_ = entity;
	result.Location_ = location;
	result.Parameters_ = tp;
	result.Mime_ = mime;
	return result;
}

LeechCraft::DownloadEntity LeechCraft::Util::MakeNotification (const QString& header,
		const QString& text, Priority priority)
{
	DownloadEntity result = MakeEntity (header,
			QString (),
			AutoAccept | OnlyHandle,
			"x-leechcraft/notification");
	result.Additional_ ["Text"] = text;
	result.Additional_ ["Priority"] = priority;
	return result;
}

QUrl LeechCraft::Util::MakeAbsoluteUrl (QUrl originalUrl, const QString& hrefUrl)
{
	if (hrefUrl.indexOf ("://") < 0)
	{
		originalUrl.setQueryItems (QList<QPair<QString, QString> > ());
		if (hrefUrl.size () &&
				hrefUrl.at (0) == '/')
			originalUrl.setEncodedPath (hrefUrl.toUtf8 ());
		else
		{
			QString originalPath = originalUrl.path ();
			if (!originalPath.endsWith ('/'))
			{
				int slashIndex = originalPath.lastIndexOf ('/');
				originalPath = originalPath.left (slashIndex + 1);
			}
			originalPath += hrefUrl;
			originalUrl.setEncodedPath (originalPath.toUtf8 ());
		}
		return originalUrl;
	}
	else
		return QUrl::fromEncoded (hrefUrl.toUtf8 ());
}

