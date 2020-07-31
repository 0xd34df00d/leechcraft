/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lackmanutil.h"
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QtDebug>
#include "repoinfo.h"

namespace LC
{
namespace LackMan
{
namespace LackManUtil
{
	QString NormalizePackageName (const QString& packageName)
	{
		QString normalized = packageName.simplified ();
		normalized.remove (' ');
		normalized.remove ('\t');
		return normalized;
	}

	InstalledDependencyInfoList GetSystemInstalledPackages (const QString& defVersion)
	{
		InstalledDependencyInfoList result;

		QFileInfoList infoEntries;
#if defined(Q_OS_WIN32)
		infoEntries += QDir (QCoreApplication::applicationDirPath () + "/share/installed")
				.entryInfoList (QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
#elif defined(Q_OS_MAC) && !defined(USE_UNIX_LAYOUT)
		infoEntries += QDir (QCoreApplication::applicationDirPath () + "/../installed")
				.entryInfoList (QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
#else
		infoEntries += QDir ("/usr/share/leechcraft/installed")
				.entryInfoList (QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
		infoEntries += QDir ("/usr/local/share/leechcraft/installed")
				.entryInfoList (QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
#endif

		QStringList entries;

		for (const auto& info : infoEntries)
		{
			const auto& path = info.absoluteFilePath ();
			if (info.isFile ())
				entries << path;
			else if (info.isDir ())
				for (const auto& subInfo : QDir { path }.entryInfoList (QDir::Files))
					entries << subInfo.absoluteFilePath ();
		}

		QString nameStart ("Name: ");
		QString versionStart ("Version: ");

		for (const auto& entry : entries)
		{
			QFile file { entry };
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to open"
						<< entry
						<< "for reading, skipping it.";
				continue;
			}

			InstalledDependencyInfo info;
			info.Source_ = InstalledDependencyInfo::SSystem;

			const auto& lines = QString { file.readAll () }.split ('\n', Qt::SkipEmptyParts);
			for (const auto& untrimmed : lines)
			{
				const auto& string = untrimmed.trimmed ();
				if (string.startsWith (nameStart))
					info.Dep_.Name_ = string.mid (nameStart.length ());
				else if (string.startsWith (versionStart))
					info.Dep_.Version_ = string.mid (versionStart.length ());
			}

			if (info.Dep_.Version_.isEmpty ())
			{
				qWarning () << Q_FUNC_INFO
						<< "dependency version for"
						<< info.Dep_.Name_
						<< "not filled, defaulting to"
						<< defVersion;
				info.Dep_.Version_ = defVersion;
			}

			result << info;
		}

		return result;
	}
}
}
}
