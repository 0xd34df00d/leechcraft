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

#include "m3ustuff.h"
#include <QFile>
#include <QTextCodec>
#include <QFileInfo>
#include <QDir>
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

		auto codec = QTextCodec::codecForName ("Windows-1252");

		QStringList result;
		while (!file.atEnd ())
		{
			const auto& line = file.readLine ().trimmed ();
			if (line.startsWith ('#'))
				continue;

			result << codec->toUnicode (line);
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

		auto codec = QTextCodec::codecForName ("Windows-1252");

		file.write (codec->fromUnicode (lines.join ("\n")));
	}

	QList<Phonon::MediaSource> Read2Sources (const QString& path)
	{
		const auto& m3uDir = QFileInfo (path).absoluteDir ();

		QList<Phonon::MediaSource> result;
		Q_FOREACH (QString src, Read (path))
		{
			QUrl url (src);
			if (!url.scheme ().isEmpty ())
			{
				result << (url.scheme () == "file" ? url.toLocalFile () : url);
				continue;
			}

			src.replace ('\\', '/');

			QFileInfo fi (src);
			if (fi.suffix () == "m3u")
				result += Read2Sources (m3uDir.absoluteFilePath (src));
			else if (fi.isRelative ())
				result << m3uDir.absoluteFilePath (src);
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
