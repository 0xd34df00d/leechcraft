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

#include "clouduploader.h"
#include <QFile>
#include <QtDebug>
#include <QFileInfo>
#include <util/util.h>
#include "../core.h"

namespace LeechCraft
{
namespace LMP
{
	CloudUploader::CloudUploader (ICloudStoragePlugin *cloud, QObject *parent)
	: QObject (parent)
	, Cloud_ (cloud)
	{
		connect (Cloud_->GetObject (),
				SIGNAL (uploadFinished (QString, LeechCraft::LMP::CloudStorageError, QString)),
				this,
				SLOT (handleUploadFinished (QString, LeechCraft::LMP::CloudStorageError, QString)),
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
			Core::Instance ().SendEntity (Util::MakeNotification ("LMP",
						tr ("Error uploading file %1 to cloud: %2.")
							.arg (QFileInfo (localPath).fileName ())
							.arg (errorStr),
						PWarning_));
	}
}
}
