/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "syncmanagerbase.h"
#include <QFileInfo>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include "transcodemanager.h"

namespace LC
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

		emit transcodingProgress (TranscodedCount_, TotalTCCount_, this);
		emit uploadProgress (CopiedCount_, TotalCopyCount_, this);

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
					Priority::Warning);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
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
				Priority::Info);
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	void SyncManagerBase::HandleFileTranscoded (const QString&, const QString&)
	{
		qDebug () << Q_FUNC_INFO << "file transcoded, gonna copy";
		emit transcodingProgress (++TranscodedCount_, TotalTCCount_, this);
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

		emit transcodingProgress (TranscodedCount_, --TotalTCCount_, this);
		CheckTCFinished ();

		emit uploadProgress (CopiedCount_, --TotalCopyCount_, this);
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

		emit uploadProgress (++CopiedCount_, TotalCopyCount_, this);
		emit singleUploadProgress (0, 0, this);
		CheckUploadFinished ();
	}

	void SyncManagerBase::handleCopyProgress (qint64 done, qint64 total)
	{
		emit singleUploadProgress (done, total, this);
	}

	void SyncManagerBase::handleErrorCopying (const QString& localPath, const QString& errorStr)
	{
		const auto& filename = QFileInfo (localPath).fileName ();
		const auto& text = tr ("Error copying file %1: %2.").arg (filename).arg (errorStr);

		GetProxyHolder ()->GetEntityManager ()->HandleEntity (Util::MakeNotification ("LMP",
					text,
					Priority::Warning));
		emit uploadLog (text);

		emit uploadProgress (++CopiedCount_, TotalCopyCount_, this);
		CheckUploadFinished ();
	}
}
}
