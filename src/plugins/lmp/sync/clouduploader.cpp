/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "clouduploader.h"
#include <QFile>
#include <QtDebug>
#include <QFileInfo>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>

namespace LC
{
namespace LMP
{
	CloudUploader::CloudUploader (ICloudStoragePlugin *cloud, QObject *parent)
	: QObject (parent)
	, Cloud_ (cloud)
	{
		connect (Cloud_->GetQObject (),
				SIGNAL (uploadFinished (QString, LC::LMP::CloudStorageError, QString)),
				this,
				SLOT (handleUploadFinished (QString, LC::LMP::CloudStorageError, QString)),
				Qt::UniqueConnection);
	}

	void CloudUploader::Upload (const UploadJob& job)
	{
		if (IsRunning ())
			Queue_ << job;
		else
			StartJob (job);
	}

	void CloudUploader::StartJob (const UploadJob& job)
	{
		qDebug () << Q_FUNC_INFO
				<< "uploading"
				<< job.Filename_
				<< "to"
				<< job.Account_;

		CurrentJob_ = job;
		Cloud_->Upload (job.Account_, job.Filename_);
		emit startedCopying (job.Filename_);
	}

	bool CloudUploader::IsRunning () const
	{
		return !CurrentJob_.Filename_.isEmpty ();
	}

	void CloudUploader::handleUploadFinished (const QString& localPath, CloudStorageError error, const QString& errorStr)
	{
		emit finishedCopying ();

		const bool remove = CurrentJob_.RemoveOnFinish_;
		CurrentJob_ = UploadJob ();

		if (!Queue_.isEmpty ())
			StartJob (Queue_.takeFirst ());

		if (error == CloudStorageError::NoError && remove)
			QFile::remove (localPath);

		if (!errorStr.isEmpty () && error != CloudStorageError::NoError)
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (Util::MakeNotification ("LMP",
						tr ("Error uploading file %1 to cloud: %2.")
							.arg (QFileInfo (localPath).fileName ())
							.arg (errorStr),
						Priority::Warning));
	}
}
}
