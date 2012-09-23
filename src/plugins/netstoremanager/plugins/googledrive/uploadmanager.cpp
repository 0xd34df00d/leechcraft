/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "uploadmanager.h"
#include <QNetworkAccessManager>
#include <QFileInfo>
#include <util/util.h>
#include "account.h"
#include "core.h"

namespace LeechCraft
{
namespace NetStoreManager
{
namespace GoogleDrive
{
	UploadManager::UploadManager (const QString& path,
			UploadType ut, const QStringList& parentid, Account *account)
	: QObject (account)
	, Account_ (account)
	, FilePath_ (path)
	, NAM_ (new QNetworkAccessManager (this))
	, UploadType_ (ut)
	, ParentId_ (parentid)
	{
		connect (Account_->GetDriveManager (),
				SIGNAL (uploadProgress (qint64, qint64, QString)),
				this,
				SLOT (handleUploadProgress (qint64, qint64, QString)));
		connect (Account_->GetDriveManager (),
				SIGNAL (uploadStatusChanged (QString, QString)),
				this,
				SLOT (handleStatusChanged (QString, QString)));
		connect (Account_->GetDriveManager (),
				SIGNAL (uploadError (QString, QString)),
				this,
				SLOT (handleError  (QString, QString)));
		connect (Account_->GetDriveManager (),
				SIGNAL (finished (QString, QString)),
				this,
				SLOT (handleFinished (QString, QString)));

		if (UploadType_ == UploadType::Upload)
			InitiateUploadSession ();
	}

	void UploadManager::InitiateUploadSession ()
	{
		Account_->GetDriveManager ()->Upload (FilePath_, ParentId_);
	}

	void UploadManager::handleError (const QString& error, const QString& filePath)
	{
		if (filePath != FilePath_)
			return;

		emit uploadError (error, FilePath_);
		deleteLater ();
	}

	void UploadManager::handleUploadProgress (qint64 sent,
			qint64 total, const QString& filePath)
	{
		if (filePath != FilePath_)
			return;

		emit uploadProgress (sent, total, FilePath_);
	}

	void UploadManager::handleStatusChanged (const QString& status,
			const QString& filePath)
	{
		if (filePath != FilePath_)
			return;

		emit uploadStatusChanged (status, FilePath_);
	}

	void UploadManager::handleFinished (const QString& id, const QString& filePath)
	{
		if (filePath != FilePath_)
			return;

		emit uploadStatusChanged (tr ("Finished"), FilePath_);
		emit finished (QStringList (id), FilePath_);
		deleteLater ();
	}
}
}
}
