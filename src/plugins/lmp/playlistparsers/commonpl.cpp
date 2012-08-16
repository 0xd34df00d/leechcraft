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

#include "commonpl.h"
#include <QFileInfo>
#include <QDir>
#include <QUrl>

namespace LeechCraft
{
namespace LMP
{
	QList<Phonon::MediaSource> CommonRead2Sources (const ReadParams& params)
	{
		const auto& plDir = QFileInfo (params.Path_).absoluteDir ();

		QList<Phonon::MediaSource> result;
		Q_FOREACH (const auto& src, params.RawParser_ (params.Path_))
		{
			QUrl url (src);
			if (!url.scheme ().isEmpty ())
			{
				result << (url.scheme () == "file" ? url.toLocalFile () : url);
				continue;
			}

			const QFileInfo fi (src);
			if (params.Suffixes_.contains (fi.suffix ()))
				result += CommonRead2Sources ({ params.Suffixes_,
							plDir.absoluteFilePath (src), params.RawParser_ });
			else if (fi.isRelative ())
				result << plDir.absoluteFilePath (src);
			else
				result << src;
		}

		return result;
	}
}
}
