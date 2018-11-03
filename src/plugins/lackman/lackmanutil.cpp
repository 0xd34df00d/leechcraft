/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "lackmanutil.h"
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QtDebug>
#include "repoinfo.h"

namespace LeechCraft
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

			const auto& lines = QString { file.readAll () }.split ('\n', QString::SkipEmptyParts);
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
