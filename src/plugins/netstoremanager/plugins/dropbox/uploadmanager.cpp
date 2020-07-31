/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "uploadmanager.h"
#include <QNetworkAccessManager>
#include <QFileInfo>
#include <util/util.h>
#include "account.h"
#include "core.h"

namespace LC
{
namespace NetStoreManager
{
namespace DBox
{
	UploadManager::UploadManager (const QString& path,
			UploadType ut, const QByteArray& parentid, Account *account,
			const QByteArray& id)
	: QObject (account)
	, Account_ (account)
	, FilePath_ (path)
	, NAM_ (new QNetworkAccessManager (this))
	, ParentId_ (parentid)
	, Id_ (id)
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
		//TODO
		connect (Account_->GetDriveManager (),
				SIGNAL (finished (QString, QString)),
				this,
				SLOT (handleFinished (QString, QString)));

		if (ut == UploadType::Upload)
			InitiateUploadSession ();
		else if (ut == UploadType::Update)
			InitiateUpdateSession ();
	}

	void UploadManager::InitiateUploadSession ()
	{
		Account_->GetDriveManager ()->Upload (FilePath_, ParentId_);
	}

	void UploadManager::InitiateUpdateSession ()
	{
		//TODO
		Account_->GetDriveManager ()->RemoveEntry (Id_.value (0).toUtf8 ());
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
		emit finished (id.toUtf8 (), FilePath_);
		deleteLater ();
	}
}
}
}
