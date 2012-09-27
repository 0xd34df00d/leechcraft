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

#include "copymanager.h"
#include <QtDebug>
#include <QFileInfo>
#include <util/util.h>
#include "interfaces/lmp/isyncplugin.h"
#include "../core.h"

namespace LeechCraft
{
namespace LMP
{
	CopyManager::CopyManager (QObject *parent)
	: QObject (parent)
	{
	}

	void CopyManager::Copy (const CopyJob& job)
	{
		if (IsRunning ())
			Queue_ << job;
		else
			StartJob (job);
	}

	void CopyManager::StartJob (const CopyJob& job)
	{
		qDebug () << Q_FUNC_INFO
				<< "copying"
				<< job.From_
				<< "to"
				<< job.MountPoint_
				<< job.Filename_;

		CurrentJob_ = job;

		connect (job.Syncer_->GetObject (),
				SIGNAL (uploadFinished (QString, QFile::FileError, QString)),
				this,
				SLOT (handleUploadFinished (QString, QFile::FileError, QString)),
				Qt::UniqueConnection);

		job.Syncer_->Upload (job.From_, job.OrigPath_, job.MountPoint_, job.Filename_);

		emit startedCopying (job.Filename_);
	}

	bool CopyManager::IsRunning () const
	{
		return !CurrentJob_.From_.isEmpty ();
	}

	void CopyManager::handleUploadFinished (const QString& localPath,
			QFile::FileError error, const QString& errorStr)
	{
		emit finishedCopying ();

		const bool remove = CurrentJob_.RemoveOnFinish_;
		CurrentJob_ = CopyJob ();

		if (!Queue_.isEmpty ())
			StartJob (Queue_.takeFirst ());

		if (error == QFile::NoError && remove)
			QFile::remove (localPath);

		if (!errorStr.isEmpty () && error != QFile::NoError)
			Core::Instance ().SendEntity (Util::MakeNotification ("LMP",
						tr ("Error uploading file %1 to cloud: %2.")
							.arg (QFileInfo (localPath).fileName ())
							.arg (errorStr),
						PWarning_));
	}
}
}
