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
	, TranscodedCount_ (0)
	, TotalTCCount_ (0)
	, WereTCErrors_ (false)
	, CopiedCount_ (0)
	, TotalCopyCount_ (0)
	{
		connect (Transcoder_,
				SIGNAL (fileStartedTranscoding (QString)),
				this,
				SLOT (handleStartedTranscoding (QString)));
		connect (Transcoder_,
				SIGNAL (fileReady (QString, QString, QString)),
				this,
				SLOT (handleFileTranscoded (QString, QString, QString)));
		connect (Transcoder_,
				SIGNAL (fileFailed (QString)),
				this,
				SLOT (handleFileTCFailed (QString)));
	}

	void SyncManager::AddFiles (ISyncPlugin *syncer, const QString& mount,
			const QStringList& files, const TranscodingParams& params)
	{
		const int numFiles = files.size ();
		TotalTCCount_ += numFiles;
		TotalCopyCount_ += numFiles;

		std::for_each (files.begin (), files.end (),
				[this, syncer, &mount] (decltype (files.front ()) file)
					{ Source2Params_ [file] = { syncer, mount }; });

		emit transcodingProgress (TranscodedCount_, TotalTCCount_);
		emit uploadProgress (CopiedCount_, TotalCopyCount_);

		Transcoder_->Enqueue (files, params);

		emit uploadLog (tr ("Uploading %n file(s)", 0, numFiles));
	}

	void SyncManager::CreateSyncer (const QString& mount)
	{
		auto mgr = new CopyManager (this);
		connect (mgr,
				SIGNAL (startedCopying (QString)),
				this,
				SLOT (handleStartedCopying (QString)));
		connect (mgr,
				SIGNAL (finishedCopying ()),
				this,
				SLOT (handleFinishedCopying ()));
		Mount2Copiers_ [mount] = mgr;
	}

	void SyncManager::CheckTCFinished ()
	{
		if (TranscodedCount_ < TotalTCCount_)
			return;

		TotalTCCount_ = 0;
		TranscodedCount_ = 0;
		WereTCErrors_ = false;
	}

	void SyncManager::CheckUploadFinished ()
	{
		if (CopiedCount_ < TotalCopyCount_)
			return;

		TotalCopyCount_ = 0;
		CopiedCount_ = 0;
	}

	namespace
	{
		bool FixMask (QString& mask, const QString& transcoded)
		{
			MediaInfo info;
			try
			{
				info = Core::Instance ().GetLocalFileResolver ()->ResolveInfo (transcoded);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< e.what ();
				return false;
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

			return true;
		}
	}

	void SyncManager::handleStartedTranscoding (const QString& file)
	{
		emit uploadLog (tr ("File %1 started transcoding...")
				.arg ("<em>" + QFileInfo (file).fileName () + "</em>"));
	}

	void SyncManager::handleFileTranscoded (const QString& from,
			const QString& transcoded, QString mask)
	{
		qDebug () << Q_FUNC_INFO << "file transcoded, gonna copy";

		emit transcodingProgress (++TranscodedCount_, TotalTCCount_);
		CheckTCFinished ();

		const auto& syncTo = Source2Params_.take (from);
		if (syncTo.MountPath_.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "dumb transcoded file detected"
					<< from
					<< transcoded;
			return;
		}

		emit uploadLog (tr ("File %1 successfully transcoded, adding to copy queue for the device %2...")
				.arg ("<em>" + QFileInfo (from).fileName () + "</em>")
				.arg ("<em>" + syncTo.MountPath_) + "</em>");

		if (!FixMask (mask, transcoded))
			return;

		if (!Mount2Copiers_.contains (syncTo.MountPath_))
			CreateSyncer (syncTo.MountPath_);
		Mount2Copiers_ [syncTo.MountPath_]->Copy ({ syncTo.Syncer_, transcoded, syncTo.MountPath_, mask });
	}

	void SyncManager::handleFileTCFailed (const QString& file)
	{
		emit uploadLog (tr ("Transcoding of file %1 failed")
				.arg ("<em>" + QFileInfo (file).fileName () + "</em>"));
		WereTCErrors_ = true;

		emit transcodingProgress (TranscodedCount_, --TotalTCCount_);
		CheckTCFinished ();

		emit uploadProgress (CopiedCount_, --TotalCopyCount_);
		CheckUploadFinished ();
	}

	void SyncManager::handleStartedCopying (const QString& file)
	{
		emit uploadLog (tr ("File %1 started copying...")
					.arg ("<em>" + QFileInfo (file).fileName () + "</em>"));
	}

	void SyncManager::handleFinishedCopying ()
	{
		emit uploadLog (tr ("File finished copying"));

		emit uploadProgress (++CopiedCount_, TotalCopyCount_);
		CheckUploadFinished ();
	}
}
}
