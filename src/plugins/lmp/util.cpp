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
#include <phonon/mediasource.h>
#include "core.h"
#include "localcollection.h"

namespace LeechCraft
{
namespace LMP
{
	QStringList RecIterate (const QString& dirPath, bool followSymlinks)
	{
		QStringList result;
		QStringList nameFilters;
		nameFilters << "*.ogg"
				<< "*.flac"
				<< "*.mp3"
				<< "*.wav";

		if (QFileInfo (dirPath).isFile ())
		{
			Q_FOREACH (const auto& filter, nameFilters)
				if (dirPath.endsWith (filter.mid (1)))
					return QStringList (dirPath);
			return QStringList ();
		}

		auto filters = QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot;
		if (!followSymlinks)
			filters |= QDir::NoSymLinks;

		const auto& list = QDir (dirPath).entryInfoList (nameFilters, filters);
		qDebug () << Q_FUNC_INFO << dirPath << list.size ();
		Q_FOREACH (const QFileInfo& entryInfo, list)
		{
			const auto& path = entryInfo.absoluteFilePath ();
			if (entryInfo.isSymLink () &&
					entryInfo.symLinkTarget () == path)
				continue;

			if (entryInfo.isDir ())
				result += RecIterate (path, followSymlinks);
			else if (entryInfo.isFile ())
				result += path;
		}

		return result;
	}

	QString FindAlbumArtPath (const QString& near, bool ignoreCollection)
	{
		if (!ignoreCollection)
		{
			auto collection = Core::Instance ().GetLocalCollection ();
			const int trackId = collection->FindTrack (near);
			if (trackId >= 0)
			{
				auto album = collection->GetTrackAlbum (trackId);
				if (!album->CoverPath_.isEmpty ())
					return album->CoverPath_;
			}
		}

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

	QPixmap FindAlbumArt (const QString& near, bool ignoreCollection)
	{
		return QPixmap (FindAlbumArtPath (near, ignoreCollection));
	}

	bool operator!= (const Phonon::MediaSource& left, const Phonon::MediaSource& right)
	{
		return !(left == right);
	}
}
}
