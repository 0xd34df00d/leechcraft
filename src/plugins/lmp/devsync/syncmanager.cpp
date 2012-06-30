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

#include "syncmanager.h"
#include <QStringList>
#include <QtDebug>
#include <QFileInfo>
#include "copymanager.h"
#include "transcodemanager.h"
#include "../core.h"
#include "../localfileresolver.h"

namespace LeechCraft
{
namespace LMP
{
	SyncManager::SyncManager (QObject *parent)
	: QObject (parent)
	, Transcoder_ (new TranscodeManager (this))
	{
		connect (Transcoder_,
				SIGNAL (fileReady (QString, QString, QString)),
				this,
				SLOT (handleFileTranscoded (QString, QString, QString)));
	}

	void SyncManager::AddFiles (ISyncPlugin *syncer, const QString& mount,
			const QStringList& files, const TranscodingParams& params)
	{
		Transcoder_->Enqueue (files, params);

		std::for_each (files.begin (), files.end (),
				[this, syncer, &mount] (decltype (files.front ()) file)
					{ Source2Params_ [file] = { syncer, mount }; });
	}

	void SyncManager::handleFileTranscoded (const QString& from,
			const QString& transcoded, QString mask)
	{
		qDebug () << Q_FUNC_INFO << "file transcoded, gonna copy";
		const auto& syncTo = Source2Params_.take (from);
		if (syncTo.MountPath_.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "dumb transcoded file detected"
					<< from
					<< transcoded;
			return;
		}

		MediaInfo info;
		try
		{
			info = Core::Instance ().GetLocalFileResolver ()->ResolveInfo (transcoded);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
			return;
		}

		mask.replace ("$artist", info.Artist_);
		mask.replace ("$year", QString::number (info.Year_));
		mask.replace ("$album", info.Album_);
		QString trackNumStr = QString::number (info.TrackNumber_);
		if (info.TrackNumber_ < 10)
			trackNumStr.prepend ('0');
		mask.replace ("$trackNumber", trackNumStr);
		mask.replace ("$title", info.Title_);

		const auto& ext = QFileInfo (transcoded).suffix ();
		if (!mask.endsWith (ext))
			mask+= "." + ext;

		if (!Mount2Copiers_.contains (syncTo.MountPath_))
			Mount2Copiers_ [syncTo.MountPath_] = new CopyManager (this);
		Mount2Copiers_ [syncTo.MountPath_]->Copy ({ syncTo.Syncer_, transcoded, syncTo.MountPath_, mask });
	}
}
}
