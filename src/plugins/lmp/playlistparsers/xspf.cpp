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

#include "xspf.h"
#include <QFile>
#include <QDomDocument>
#include <QFileInfo>
#include <QDir>
#include <QUrl>

namespace LeechCraft
{
namespace LMP
{
namespace XSPF
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

		QDomDocument doc;
		if (!doc.setContent (file.readAll ()))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse"
					<< path;
			return QStringList ();
		}

		QStringList result;
		auto track = doc.documentElement ()
				.firstChildElement ("trackList")
				.firstChildElement ("track");
		while (!track.isNull ())
		{
			const auto& loc = track.firstChildElement ("location").text ();
			if (!loc.isEmpty ())
				result << loc;

			track = track.nextSiblingElement ("track");
		}
		return result;
	}

	QList<Phonon::MediaSource> Read2Sources (const QString& path)
	{
		const auto& plDir = QFileInfo (path).absoluteDir ();

		QList<Phonon::MediaSource> result;
		Q_FOREACH (const auto& src, Read (path))
		{
			QUrl url (src);
			if (!url.scheme ().isEmpty ())
			{
				result << (url.scheme () == "file" ? url.toLocalFile () : url);
				continue;
			}

			const QFileInfo fi (src);
			if (fi.suffix () == "xspf")
				result += Read2Sources (plDir.absoluteFilePath (src));
			else if (fi.isRelative ())
				result << plDir.absoluteFilePath (src);
			else
				result << src;
		}

		return result;
	}
}
}
}
