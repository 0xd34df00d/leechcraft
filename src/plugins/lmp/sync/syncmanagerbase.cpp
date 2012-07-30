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

#include "syncmanagerbase.h"
#include <util/util.h>
#include "transcodemanager.h"
#include "../core.h"

namespace LeechCraft
{
namespace LMP
{
	SyncManagerBase::SyncManagerBase (QObject *parent)
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

	void SyncManagerBase::AddFiles (const QStringList& files, const TranscodingParams& params)
	{
		const int numFiles = files.size ();
		TotalTCCount_ += numFiles;
		TotalCopyCount_ += numFiles;

		emit transcodingProgress (TranscodedCount_, TotalTCCount_);
		emit uploadProgress (CopiedCount_, TotalCopyCount_);

		Transcoder_->Enqueue (files, params);

		emit uploadLog (tr ("Uploading %n file(s)", 0, numFiles));
	}

	void SyncManagerBase::CheckTCFinished ()
	{
		if (TranscodedCount_ < TotalTCCount_)
			return;

		if (WereTCErrors_)
		{
			const auto& e = Util::MakeNotification ("LMP",
					tr ("Files were transcoded, but some errors occured. "
						"Check the upload log for details."),
					PWarning_);
			Core::Instance ().SendEntity (e);
			WereTCErrors_ = false;
		}

		TotalTCCount_ = 0;
		TranscodedCount_ = 0;
	}

	void SyncManagerBase::CheckUploadFinished ()
	{
		if (CopiedCount_ < TotalCopyCount_)
			return;

		TotalCopyCount_ = 0;
		CopiedCount_ = 0;

		const auto& e = Util::MakeNotification ("LMP",
				tr ("Files finished uploading."),
				PInfo_);
		Core::Instance ().SendEntity (e);
	}

	void SyncManagerBase::HandleFileTranscoded (const QString&, const QString&)
	{
		qDebug () << Q_FUNC_INFO << "file transcoded, gonna copy";
		emit transcodingProgress (++TranscodedCount_, TotalTCCount_);
		CheckTCFinished ();
	}

	void SyncManagerBase::handleStartedTranscoding (const QString& file)
	{
		emit uploadLog (tr ("File %1 started transcoding...")
				.arg ("<em>" + QFileInfo (file).fileName () + "</em>"));
	}

	void SyncManagerBase::handleFileTCFailed (const QString& file)
	{
		emit uploadLog (tr ("Transcoding of file %1 failed")
				.arg ("<em>" + QFileInfo (file).fileName () + "</em>"));
		WereTCErrors_ = true;

		emit transcodingProgress (TranscodedCount_, --TotalTCCount_);
		CheckTCFinished ();

		emit uploadProgress (CopiedCount_, --TotalCopyCount_);
		CheckUploadFinished ();
	}

	void SyncManagerBase::handleStartedCopying (const QString& file)
	{
		emit uploadLog (tr ("File %1 started copying...")
					.arg ("<em>" + QFileInfo (file).fileName () + "</em>"));
	}

	void SyncManagerBase::handleFinishedCopying ()
	{
		emit uploadLog (tr ("File finished copying"));

		emit uploadProgress (++CopiedCount_, TotalCopyCount_);
		CheckUploadFinished ();
	}
}
}
