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

#include "m3u.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QtDebug>

namespace LeechCraft
{
namespace LMP
{
namespace M3U
{
	QStringList Read (const QString& path)
	{
		QFile file (path);
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open"
					<< path
					<< file.errorString ();
			return QStringList ();
		}

		QStringList result;
		while (!file.atEnd ())
		{
			const auto& line = file.readLine ().trimmed ();
			if (line.startsWith ('#'))
				continue;

			result << QString::fromUtf8 (line.constData ());
		}
		return result;
	}

	void Write (const QString& path, const QStringList& lines)
	{
		QFile file (path);
		if (!file.open (QIODevice::WriteOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open"
					<< path
					<< file.errorString ();
			return;
		}

		file.write (lines.join ("\n").toUtf8 ());
	}

	QList<Phonon::MediaSource> Read2Sources (const QString& path)
	{
		const auto& m3uDir = QFileInfo (path).absoluteDir ();

		QList<Phonon::MediaSource> result;
		Q_FOREACH (QString src, Read (path))
		{
			QUrl url (src);
#ifdef Q_OS_WIN32
			if (url.scheme ().size () > 1)
#else
			if (!url.scheme ().isEmpty ())
#endif
			{
				result << (url.scheme () == "file" ? url.toLocalFile () : url);
				continue;
			}

			src.replace ('\\', '/');

			const QFileInfo fi (src);
			if (fi.isRelative ())
				src = m3uDir.absoluteFilePath (src);

			if (fi.suffix () == "m3u" || fi.suffix () == "m3u8")
				result += Read2Sources (src);
			else
				result << src;
		}
		return result;
	}

	void Write (const QString& path, const QList<Phonon::MediaSource>& sources)
	{
		QStringList strings;
		Q_FOREACH (const Phonon::MediaSource& source, sources)
		{
			switch (source.type ())
			{
			case Phonon::MediaSource::LocalFile:
				strings << source.fileName ();
				break;
			case Phonon::MediaSource::Url:
				strings << source.url ().toString ();
				break;
			default:
				break;
			}
		}
		Write (path, strings);
	}
}
}
}
