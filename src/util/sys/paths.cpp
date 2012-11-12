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

#include "paths.h"
#include <QFile>
#if defined (Q_OS_WIN32) || defined (Q_OS_MAC)
#include <QApplication>
#endif
#include <QtDebug>

namespace LeechCraft
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
#ifdef Q_OS_WIN32
			candidates << QApplication::applicationDirPath () + "/share/qml/" + suffix;
#elif defined (Q_OS_MAC)
			candidates << QApplication::applicationDirPath () + "/../Resources/share/qml/" + suffix;
#else
			candidates << "/usr/local/share/leechcraft/qml/" + suffix
					<< "/usr/share/leechcraft/qml/" + suffix;
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
		for (const QString& cand : GetPathCandidates (SysPath::QML, suffix))
			if (QFile::exists (cand + filename))
				return cand + filename;

		qWarning () << Q_FUNC_INFO
				<< "unable to find"
				<< suffix
				<< filename;
		return QString ();
	}
}
}
