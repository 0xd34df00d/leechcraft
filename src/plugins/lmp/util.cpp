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

#include "util.h"
#include <algorithm>
#include <QDirIterator>
#include <QPixmap>

namespace LeechCraft
{
namespace LMP
{
	QStringList RecIterate (const QString& dirPath)
	{
		QStringList result;
		QStringList nameFilters;
		nameFilters << ".ogg"
				<< ".flac"
				<< ".mp3"
				<< ".wav";
		QDirIterator iterator (dirPath, QDirIterator::Subdirectories);
		while (iterator.hasNext ())
		{
			const QString& path = iterator.next ();
			Q_FOREACH (const QString& name, nameFilters)
				if (path.endsWith (name, Qt::CaseInsensitive))
				{
					result << path;
					break;
				}
		}
		return result;
	}

	QString FindAlbumArtPath (const QString& near)
	{
		QStringList possibleBases;
		possibleBases << "cover" << "folder" << "front";

		const QDir& dir = QFileInfo (near).absoluteDir ();
		const QStringList& entryList = dir.entryList (QStringList ("*.jpg") << "*.png" << "*.bmp");
		auto pos = std::find_if (entryList.begin (), entryList.end (),
				[&possibleBases] (const QString& name)
				{
					Q_FOREACH (const QString& pBase, possibleBases)
						if (name.startsWith (pBase, Qt::CaseInsensitive))
							return true;
					return false;
				});
		return pos == entryList.end () ? QString () : dir.filePath (*pos);
	}

	QPixmap FindAlbumArt (const QString& near)
	{
		return QPixmap (FindAlbumArtPath (near));
	}
}
}
