/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "paths.h"
#include <stdexcept>
#include <filesystem>
#include <QFile>
#include <QTemporaryFile>
#if defined (Q_OS_WIN32) || defined (Q_OS_MAC)
#include <QApplication>
#endif
#include <QtDebug>
#include <QDir>
#include <QUrl>
#include <QStandardPaths>
#include <util/util.h>

namespace LC
{
namespace Util
{
	QStringList GetPathCandidates (SysPath path, QString suffix)
	{
		if (!suffix.isEmpty () && suffix.at (suffix.size () - 1) != '/')
			suffix += '/';

		QStringList candidates;
		switch (path)
		{
		case SysPath::QML:
			return GetPathCandidates (SysPath::Share, "qml5/" + suffix);
		case SysPath::Share:
#ifdef Q_OS_WIN32
			candidates << QApplication::applicationDirPath () + "/share/" + suffix;
#elif defined (Q_OS_MAC) && !defined (USE_UNIX_LAYOUT)
			candidates << QApplication::applicationDirPath () + "/../Resources/share/" + suffix;
#else
	#ifdef INSTALL_PREFIX
			candidates << INSTALL_PREFIX "/share/leechcraft/" + suffix;
	#endif
			candidates << "/usr/local/share/leechcraft/" + suffix
					<< "/usr/share/leechcraft/" + suffix;
#endif
			return candidates;
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown system path"
				<< static_cast<int> (path);
		return QStringList ();
	}

	QString GetSysPath (SysPath path, const QString& suffix, const QString& filename)
	{
		for (const QString& cand : GetPathCandidates (path, suffix))
			if (QFile::exists (cand + filename))
				return cand + filename;

		qWarning () << Q_FUNC_INFO
				<< "unable to find"
				<< suffix
				<< filename;
		return QString ();
	}

	QUrl GetSysPathUrl (SysPath path, const QString& subfolder, const QString& filename)
	{
		return QUrl::fromLocalFile (GetSysPath (path, subfolder, filename));
	}

	QStringList GetSystemPaths ()
	{
		return QString (qgetenv ("PATH")).split (":", Qt::SkipEmptyParts);
	}

	QString FindInSystemPath (const QString& name, const QStringList& paths,
			const std::function<bool (QFileInfo)>& filter)
	{
		for (const auto& dir : paths)
		{
			const QFileInfo fi (dir + '/' + name);
			if (!fi.exists ())
				continue;

			if (filter && !filter (fi))
				continue;

			return fi.absoluteFilePath ();
		}

		return {};
	}

	QDir GetUserDir (UserDir dir, const QString& subpath)
	{
		QString path;
		switch (dir)
		{
		case UserDir::Cache:
			path = QStandardPaths::writableLocation (QStandardPaths::CacheLocation);
			break;
		case UserDir::LC:
			path = QDir::home ().path () + "/.leechcraft/";
			break;
		}

		if (path.isEmpty ())
			throw std::runtime_error ("cannot get root path");

		if (!path.endsWith ('/'))
			path += '/';
		if (dir == UserDir::Cache)
			path += "leechcraft5/";
		path += subpath;

		if (!QDir {}.exists (path) &&
				!QDir {}.mkpath (path))
			throw std::runtime_error ("cannot create path " + path.toStdString ());

		return { path };
	}

	QDir CreateIfNotExists (QString path)
	{
		auto home = QDir::home ();
		path.prepend (".leechcraft/");

		if (!home.exists (path) &&
				!home.mkpath (path))
			throw std::runtime_error (qPrintable (QObject::tr ("Could not create %1")
						.arg (QDir::toNativeSeparators (home.filePath (path)))));

		if (home.cd (path))
			return home;
		else
			throw std::runtime_error (qPrintable (QObject::tr ("Could not cd into %1")
						.arg (QDir::toNativeSeparators (home.filePath (path)))));
	}

	QString GetTemporaryName (const QString& pattern)
	{
		static const auto defaultPattern = QStringLiteral ("lc_temp.XXXXXX");
		QTemporaryFile file (QDir::tempPath () + '/' + (pattern.isEmpty () ? defaultPattern : pattern));
		file.open ();
		QString name = file.fileName ();
		file.close ();
		file.remove ();
		return name;
	}

	SpaceInfo GetSpaceInfo (const QString& path)
	{
		const auto& info = std::filesystem::space (path.toStdString ());
		return
		{
			info.capacity,
			info.free,
			info.available
		};
	}
}
}
