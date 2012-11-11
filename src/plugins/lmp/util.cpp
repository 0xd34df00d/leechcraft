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
#include <QTimer>
#include <QPixmap>
#include <QDesktopWidget>
#include <QLabel>
#include <QApplication>
#include <QKeyEvent>
#include <phonon/mediasource.h>
#include "core.h"
#include "localcollection.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace LMP
{
	QStringList RecIterate (const QString& dirPath, bool followSymlinks)
	{
		QStringList result;
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

		if (QFileInfo (dirPath).isFile ())
		{
			Q_FOREACH (const auto& filter, nameFilters)
				if (dirPath.endsWith (filter.mid (1), Qt::CaseInsensitive))
					return QStringList (dirPath);
			return QStringList ();
		}

		auto filters = QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot;
		if (!followSymlinks)
			filters |= QDir::NoSymLinks;

		const auto& list = QDir (dirPath).entryInfoList (nameFilters, filters);
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

		return QPixmap (FindAlbumArtPath (near, ignoreCollection));
	}

	namespace
	{
		class AADisplayEventFilter : public QObject
		{
			QWidget *Display_;
		public:
			AADisplayEventFilter (QWidget *display)
			: QObject (display)
			, Display_ (display)
			{
			}
		protected:
			bool eventFilter (QObject*, QEvent *event)
			{
				bool shouldClose = false;
				switch (event->type ())
				{
				case QEvent::KeyRelease:
					shouldClose = static_cast<QKeyEvent*> (event)->key () == Qt::Key_Escape;
					break;
				case QEvent::MouseButtonRelease:
					shouldClose = true;
					break;
				default:
					break;
				}

				if (!shouldClose)
					return false;

				QTimer::singleShot (0,
						Display_,
						SLOT (close ()));
				return true;
			}
		};
	}

	void ShowAlbumArt (const QString& near, const QPoint& pos)
	{
		auto px = FindAlbumArt (near);
		if (px.isNull ())
			return;

		const auto& availGeom = QApplication::desktop ()->availableGeometry (pos).size () * 0.9;
		if (px.size ().width () > availGeom.width () ||
			px.size ().height () > availGeom.height ())
			px = px.scaled (availGeom, Qt::KeepAspectRatio, Qt::SmoothTransformation);

		auto label = new QLabel;
		label->setWindowTitle (QObject::tr ("Album art"));
		label->setWindowFlags (Qt::Tool);
		label->setAttribute (Qt::WA_DeleteOnClose);
		label->setFixedSize (px.size ());
		label->setPixmap (px);
		label->show ();
		label->activateWindow ();
		label->installEventFilter (new AADisplayEventFilter (label));
	}

	QString PerformSubstitutions (QString mask, const MediaInfo& info)
	{
		mask.replace ("$artist", info.Artist_);
		mask.replace ("$year", QString::number (info.Year_));
		mask.replace ("$album", info.Album_);
		QString trackNumStr = QString::number (info.TrackNumber_);
		if (info.TrackNumber_ < 10)
			trackNumStr.prepend ('0');
		mask.replace ("$trackNumber", trackNumStr);
		mask.replace ("$title", info.Title_);
		return mask;
	}

	bool ShouldRememberProvs ()
	{
		return XmlSettingsManager::Instance ().property ("RememberUsedProviders").toBool ();
	}

	bool operator!= (const Phonon::MediaSource& left, const Phonon::MediaSource& right)
	{
		return !(left == right);
	}
}
}
