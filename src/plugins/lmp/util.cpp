/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
#include <QApplication>
#include <QLabel>
#include <phonon/mediasource.h>
#include <util/util.h>
#include <util/gui/util.h>
#include "core.h"
#include "localcollection.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace LMP
{
	QList<QFileInfo> RecIterateInfo (const QString& dirPath, bool followSymlinks)
	{
		QStringList nameFilters;
		nameFilters << "*.aiff"
				<< "*.ape"
				<< "*.asf"
				<< "*.flac"
				<< "*.m4a"
				<< "*.mp3"
				<< "*.mp4"
				<< "*.mpc"
				<< "*.mpeg"
				<< "*.mpg"
				<< "*.ogg"
				<< "*.tta"
				<< "*.wav"
				<< "*.wma"
				<< "*.wv"
				<< "*.wvp";

		const QFileInfo dirInfo (dirPath);
		if (dirInfo.isFile ())
		{
			Q_FOREACH (const auto& filter, nameFilters)
				if (dirPath.endsWith (filter.mid (1), Qt::CaseInsensitive))
					return { dirInfo };
			return QList<QFileInfo> ();
		}

		auto filters = QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot;
		if (!followSymlinks)
			filters |= QDir::NoSymLinks;

		QList<QFileInfo> result;
		const auto& list = QDir (dirPath).entryInfoList (nameFilters, filters);
		Q_FOREACH (const QFileInfo& entryInfo, list)
		{
			const auto& path = entryInfo.absoluteFilePath ();
			if (entryInfo.isSymLink () &&
					entryInfo.symLinkTarget () == path)
				continue;

			if (entryInfo.isDir ())
				result += RecIterateInfo (path, followSymlinks);
			else if (entryInfo.isFile ())
				result += entryInfo;
		}

		return result;
	}

	QStringList RecIterate (const QString& dirPath, bool followSymlinks)
	{
		const auto& infos = RecIterateInfo (dirPath, followSymlinks);
		QStringList result;
		result.reserve (infos.size ());
		for (const auto& info : infos)
			result << info.absoluteFilePath ();
		return result;
	}

	QString FindAlbumArtPath (const QString& near, bool ignoreCollection)
	{
		if (near.isEmpty ())
			return QString ();

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
				[&possibleBases] (const QString& name) -> bool
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
		if (near.isEmpty ())
			return QPixmap ();

		const QPixmap nearPx (near);
		if (!nearPx.isNull ())
			return nearPx;

		return QPixmap (FindAlbumArtPath (near, ignoreCollection));
	}

	void ShowAlbumArt (const QString& near, const QPoint& pos)
	{
		auto px = FindAlbumArt (near);
		if (px.isNull ())
			return;

		auto label = Util::ShowPixmapLabel (px, pos);
		label->setWindowTitle (QObject::tr ("Album art"));
	}

	QMap<QString, std::function<QString (MediaInfo)>> GetSubstGetters ()
	{
		return Util::MakeMap<QString, std::function<QString (MediaInfo)>> ({
				{ "$artist", [] (const MediaInfo& info) { return info.Artist_; } },
				{ "$album", [] (const MediaInfo& info) { return info.Album_; } },
				{ "$title", [] (const MediaInfo& info) { return info.Title_; } },
				{ "$year", [] (const MediaInfo& info) { return QString::number (info.Year_); } },
				{ "$trackNumber", [] (const MediaInfo& info) -> QString
					{
						auto trackNumStr = QString::number (info.TrackNumber_);
						if (info.TrackNumber_ < 10)
							trackNumStr.prepend ('0');
						return trackNumStr;
					} }
			});
	}

	QMap<QString, std::function<void (MediaInfo&, QString)>> GetSubstSetters ()
	{
		return Util::MakeMap<QString, std::function<void (MediaInfo&, QString)>> ({
				{ "$artist", [] (MediaInfo& info, const QString& val) { info.Artist_ = val; } },
				{ "$album", [] (MediaInfo& info, const QString& val) { info.Album_= val; } },
				{ "$title", [] (MediaInfo& info, const QString& val) { info.Title_ = val; } },
				{ "$year", [] (MediaInfo& info, const QString& val) { info.Year_ = val.toInt (); } },
				{ "$trackNumber", [] (MediaInfo& info, QString val)
					{
						if (val.size () == 2 && val.at (0) == '0')
							val = val.mid (1);
						info.TrackNumber_ = val.toInt ();
					} }
			});
	}

	QString PerformSubstitutions (QString mask, const MediaInfo& info)
	{
		const auto& getters = GetSubstGetters ();
		for (const auto& key : getters.keys ())
			mask.replace (key, getters [key] (info));
		return mask;
	}

	bool ShouldRememberProvs ()
	{
		return XmlSettingsManager::Instance ().property ("RememberUsedProviders").toBool ();
	}

	QString MakeTrackListTooltip (const QList<QList<Media::ReleaseTrackInfo>>& infos)
	{
		QString trackTooltip;
		int mediumPos = 0;
		for (const auto& medium : infos)
		{
			if (infos.size () > 1)
			{
				if (mediumPos)
					trackTooltip += "<br />";
				trackTooltip += QObject::tr ("CD %1:").arg (++mediumPos) + "<br />";
			}

			for (const auto& track : medium)
			{
				trackTooltip += QString::number (track.Number_) + ". ";
				trackTooltip += track.Name_;
				if (track.Length_)
				{
					auto lengthStr = Util::MakeTimeFromLong (track.Length_);
					if (lengthStr.startsWith ("00:"))
						lengthStr = lengthStr.mid (3);
					trackTooltip += " (" + lengthStr + ")";
				}
				trackTooltip += "<br/>";
			}
		}
		return trackTooltip;
	}

	bool operator!= (const Phonon::MediaSource& left, const Phonon::MediaSource& right)
	{
		return !(left == right);
	}
}
}
