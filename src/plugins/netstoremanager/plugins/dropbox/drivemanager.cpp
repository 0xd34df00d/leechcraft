/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "drivemanager.h"
#include <QNetworkRequest>
#include <QtDebug>
#include <QDir>
#include <QFileInfo>
#include <QDesktopServices>
#include <QMessageBox>
#include <QMainWindow>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/xpc/util.h>
#include <util/sll/parsejson.h>
#include <util/sll/either.h>
#include <util/util.h>
#include <util/threads/futures.h>
#include "account.h"
#include "chunkiodevice.h"
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace NetStoreManager
{
namespace DBox
{
	StorageItem ToStorageItem (const DBoxItem& item)
	{
		StorageItem storageItem;
		storageItem.ID_ = item.Id_.toUtf8 ();
		storageItem.ParentID_ = item.ParentID_.toUtf8 ();
		storageItem.Name_ = item.Name_;
		storageItem.Size_ = item.FileSize_;
		storageItem.ModifyDate_ = item.ModifiedDate_;
		storageItem.IsDirectory_ = item.IsFolder_;
		storageItem.IsTrashed_ = item.IsDeleted_;
		storageItem.MimeType_ = item.MimeType_;
		storageItem.Hash_ = item.Revision_;

		return storageItem;
	}

	namespace
	{
		const int ChunkUploadBound = 150 * 1024 * 1024;
	}

	DriveManager::DriveManager (Account *acc, QObject *parent)
	: QObject (parent)
	, DirectoryId_ ("application/vnd.google-apps.folder")
	, Account_ (acc)
	, SecondRequestIfNoItems_ (true)
	{
	}

	void DriveManager::RequestUserId ()
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] () { RequestAccountInfo (); };
	}

	QFuture<DriveManager::RefreshResult_t> DriveManager::RefreshListing (const QByteArray& parentId)
	{
		auto guard = MakeRunnerGuard ();

		QFutureInterface<RefreshResult_t> iface;
		iface.reportStarted ();

		ApiCallQueue_ << [this, parentId, iface] () { RequestFiles (parentId, iface); };

		return iface.future ();
	}

	QFuture<DriveManager::ShareResult_t> DriveManager::ShareEntry (const QString& id, ShareType type)
	{
		if (id.isEmpty ())
			throw std::runtime_error { std::string { Q_FUNC_INFO } + ": empty id" };

		QFutureInterface<ShareResult_t> iface;

		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this, id, type, iface] { RequestSharingEntry (id, type, iface); };

		return iface.future ();
	}

	void DriveManager::CreateDirectory (const QString& name, const QString& parentId)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this, name, parentId] () { RequestCreateDirectory (name, parentId); };
	}

	void DriveManager::RemoveEntry (const QByteArray& id)
	{
		if (id.isEmpty ())
			return;
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this, id] () { RequestEntryRemoving (id); };
	}

	void DriveManager::Copy (const QByteArray& id, const QString& parentId)
	{
		if (id.isEmpty ())
			return;
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this, id, parentId] () { RequestCopyItem (id, parentId); };
	}

	void DriveManager::Move (const QByteArray& id, const QString& parentId)
	{
		if (id.isEmpty ())
			return;
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this, id, parentId] () { RequestMoveItem (id, parentId); };
	}

	void DriveManager::Upload (const QString& filePath, const QStringList& parentId)
	{
		QString parent = parentId.value (0);
		auto guard = MakeRunnerGuard ();

		if (QFileInfo (filePath).size () < ChunkUploadBound)
			ApiCallQueue_ << [this, filePath, parent] () { RequestUpload (filePath, parent); };
		else
			ApiCallQueue_ << [this, filePath, parent] () { RequestChunkUpload (filePath, parent); };
	}

	std::shared_ptr<void> DriveManager::MakeRunnerGuard ()
	{
		const bool shouldRun = ApiCallQueue_.isEmpty ();
		return std::shared_ptr<void> (nullptr, [this, shouldRun] (void*)
			{
				if (shouldRun)
					ApiCallQueue_.dequeue () ();
			});
	}

	void DriveManager::RequestAccountInfo ()
	{
		if (Account_->GetAccessToken ().isEmpty ())
			return;

		QString str = QString ("https://api.dropbox.com/1/account/info?access_token=%1")
				.arg (Account_->GetAccessToken ());
		QNetworkRequest request (str);
		request.setHeader (QNetworkRequest::ContentTypeHeader,
				"application/x-www-form-urlencoded");
		QNetworkReply *reply = Core::Instance ().GetProxy ()->
				GetNetworkAccessManager ()->get (request);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotAccountInfo ()));
	}

	namespace
	{
		DBoxItem CreateDBoxItem (const QVariant& itemData)
		{
			const QVariantMap& map = itemData.toMap ();

			DBoxItem driveItem;
			driveItem.FileSize_ = map ["bytes"].toULongLong ();
			driveItem.FolderHash_ = map ["hash"].toString ();
			driveItem.Revision_ = map ["rev"].toByteArray ();
			const auto& path = map ["path"].toString ();
			driveItem.Id_ = path;
			const auto& parent = QFileInfo (path).dir ().absolutePath ();
			driveItem.ParentID_ = parent == "/" ? QString () : parent;
			driveItem.IsDeleted_ = map ["is_deleted"].toBool ();
			driveItem.IsFolder_ = map ["is_dir"].toBool ();
			driveItem.ModifiedDate_ = map ["modified"].toDateTime ();
			driveItem.Name_ = QFileInfo (path).fileName ();
			driveItem.MimeType_ = map ["mime_type"].toString ().replace ('/', '-');

			return driveItem;
		}
	}

	void DriveManager::RequestFiles (const QByteArray& parentId, QFutureInterface<RefreshResult_t> iface)
	{
		if (Account_->GetAccessToken ().isEmpty ())
		{
			Util::ReportFutureResult (iface, Util::Left { tr ("No access token for this account.") });
			return;
		}

		QString str = QString ("https://api.dropbox.com/1/metadata/dropbox?access_token=%1&path=%2")
				.arg (Account_->GetAccessToken ())
				.arg (parentId.isEmpty () ? "/" : QString::fromUtf8 (parentId));
		QNetworkRequest request (str);

		request.setHeader (QNetworkRequest::ContentTypeHeader,
				"application/x-www-form-urlencoded");

		QNetworkReply *reply = Core::Instance ().GetProxy ()->
				GetNetworkAccessManager ()->get (request);

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[=, this] () mutable
			{
				reply->deleteLater ();

				const auto& res = Util::ParseJson (reply, Q_FUNC_INFO);
				if (res.isNull ())
				{
					Util::ReportFutureResult (iface, Util::Left { tr ("Unable to parse server reply.") });
					return;
				}

				const auto& resMap = res.toMap ();
				if (!resMap.contains ("contents"))
				{
					if (SecondRequestIfNoItems_)
					{
						SecondRequestIfNoItems_ = false;
						RequestFiles ({}, iface);
					}
					else
						Util::ReportFutureResult (iface, Util::Left { tr ("Server returned empty files info.") });
					return;
				}

				SecondRequestIfNoItems_ = true;
				QList<StorageItem> resList;
				for (const auto& item : resMap ["contents"].toList ())
				{
					const auto& driveItem = CreateDBoxItem (item);
					if (!driveItem.Name_.isEmpty ())
						resList << ToStorageItem (driveItem);
				}

				Util::ReportFutureResult (iface, resList);
			},
			reply,
			SIGNAL (finished ()),
			reply
		};
	}

	void DriveManager::RequestSharingEntry (const QString& id, ShareType type, QFutureInterface<ShareResult_t> iface)
	{
		QString str;
		switch (type)
		{
		case ShareType::Preview:
			str = QString ("https://api.dropbox.com/1/media/dropbox/%1?access_token=%2")
					.arg (id)
					.arg (Account_->GetAccessToken ());
			break;
		case ShareType::Share:
			str = QString ("https://api.dropbox.com/1/shares/dropbox/%1?access_token=%2")
					.arg (id)
					.arg (Account_->GetAccessToken ());
			break;
		}

		QNetworkRequest request (str);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");

		const auto reply = Core::Instance ().GetProxy ()->
				GetNetworkAccessManager ()->post (request, QByteArray ());

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[reply, iface] () mutable
			{
				reply->deleteLater ();

				const auto& res = Util::ParseJson (reply, Q_FUNC_INFO);
				Util::ReportFutureResult (iface,
						res.isNull () ?
							ShareResult_t { Util::AsLeft, tr ("Unable to parse server reply.") } :
							ShareResult_t { res.toMap () ["url"].toUrl () });
			},
			reply,
			SIGNAL (finished ()),
			reply
		};
	}

	void DriveManager::RequestCreateDirectory (const QString& name, const QString& parentId)
	{
		QString str = QString ("https://api.dropbox.com/1/fileops/create_folder?access_token=%1&root=%2&path=%3")
				.arg (Account_->GetAccessToken ())
				.arg ("dropbox")
				.arg ((parentId.isEmpty () ? "/" : parentId) + "/" + name);

		QNetworkRequest request (str);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");
		QNetworkReply *reply = Core::Instance ().GetProxy ()->GetNetworkAccessManager ()->
				post (request, QByteArray ());
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleCreateDirectory ()));
	}

	void DriveManager::RequestEntryRemoving (const QString& id)
	{
		QString str = QString ("https://api.dropbox.com/1/fileops/delete?access_token=%1&root=%2&path=%3")
				.arg (Account_->GetAccessToken ())
				.arg ("dropbox")
				.arg (id);
		QNetworkRequest request (str);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");

		QNetworkReply *reply = Core::Instance ().GetProxy ()->
				GetNetworkAccessManager ()->post (request, QByteArray ());

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleRequestEntryRemoving ()));
	}

	void DriveManager::RequestCopyItem (const QString& id, const QString& parentId)
	{
		const auto& parentID = parentId.isEmpty () ? "/" : parentId;
		QString str = QString ("https://api.dropbox.com/1/fileops/copy?access_token=%1&root=%2&from_path=%3&to_path=%4")
				.arg (Account_->GetAccessToken ())
				.arg ("dropbox")
				.arg (id)
				.arg (parentID + "/" + QFileInfo (id).fileName ());

		QNetworkRequest request (str);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");
		QNetworkReply *reply = Core::Instance ().GetProxy ()->GetNetworkAccessManager ()->
				post (request, QByteArray ());
		Reply2Id_ [reply] = parentID;
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleCopyItem ()));
	}

	void DriveManager::RequestMoveItem (const QString& id, const QString& parentId)
	{
		const auto& parentID = parentId.isEmpty () ? "/" : parentId;
		QString str = QString ("https://api.dropbox.com/1/fileops/move?access_token=%1&root=%2&from_path=%3&to_path=%4")
				.arg (Account_->GetAccessToken ())
				.arg ("dropbox")
				.arg (id)
				.arg (parentID + "/" + QFileInfo (id).fileName ());

		QNetworkRequest request (str);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");
		QNetworkReply *reply = Core::Instance ().GetProxy ()->GetNetworkAccessManager ()->
				post (request, QByteArray ());
		Reply2Id_ [reply] = parentID;
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleMoveItem ()));
	}

	void DriveManager::RequestUpload (const QString& filePath, const QString& parent)
	{
		QFile *file = new QFile (filePath);
		if (!file->open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file: "
					<< file->errorString ();
			return;
		}
		emit uploadStatusChanged (tr ("Uploading..."), filePath);
		QFileInfo info (filePath);

		const QUrl url (QString ("https://api-content.dropbox.com/1/files_put/%1/%2?access_token=%3")
				.arg ("dropbox")
				.arg ((parent.isEmpty () ? "/" : parent) + "/" + info.fileName ())
				.arg (Account_->GetAccessToken ()));
		QNetworkRequest request (url);
		request.setPriority (QNetworkRequest::LowPriority);
		request.setHeader (QNetworkRequest::ContentLengthHeader, info.size ());
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");

		QNetworkReply *reply = Core::Instance ().GetProxy ()->
				GetNetworkAccessManager ()->put (request, file);
		file->setParent (reply);
		Reply2FilePath_ [reply] = filePath;
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleUploadFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleUploadError (QNetworkReply::NetworkError)));
		connect (reply,
				SIGNAL (uploadProgress (qint64, qint64)),
				this,
				SLOT (handleUploadProgress (qint64, qint64)));
	}

	void DriveManager::RequestChunkUpload (const QString& filePath, const QString& parent,
			const QString& uploadId, quint64 offset)
	{
		ChunkIODevice *chunkFile = new ChunkIODevice (filePath, this);
		if (!chunkFile->open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file: "
					<< chunkFile->errorString ();
			return;
		}
		emit uploadStatusChanged (tr ("Uploading..."), filePath);

		QFileInfo info (filePath);
		QUrl url;
		if (!chunkFile->atEnd () && uploadId.isEmpty ())
			url = QString ("https://api-content.dropbox.com/1/chunked_upload?access_token=%1")
					.arg (Account_->GetAccessToken ());
		else if (!chunkFile->atEnd ())
			url = QString ("https://api-content.dropbox.com/1/chunked_upload?access_token=%1&upload_id=%2&offset=%3")
					.arg (Account_->GetAccessToken ())
					.arg (uploadId)
					.arg (offset);
		else
			url = QString ("https://api-content.dropbox.com/1/commit_chunked_upload/%1/%2?access_token=%3&upload_id=%4")
					.arg ("dropbox")
					.arg ((parent.isEmpty () ? "/" : parent) + "/" + info.fileName ())
					.arg (Account_->GetAccessToken ())
					.arg (uploadId);

		QNetworkRequest request (url);
		request.setPriority (QNetworkRequest::LowPriority);
		request.setHeader (QNetworkRequest::ContentLengthHeader, info.size ());
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");

		QNetworkReply *reply = Core::Instance ().GetProxy ()->
				GetNetworkAccessManager ()->put (request, chunkFile->GetNextChunk ());
		Reply2FilePath_ [reply] = filePath;
		Reply2ParentId_ [reply] = parent.isEmpty () ? "/" : parent;
		if (offset)
			Reply2Offset_ [reply] = offset;
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleChunkUploadFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleUploadError (QNetworkReply::NetworkError)));
		connect (reply,
				SIGNAL (uploadProgress (qint64, qint64)),
				this,
				SLOT (handleUploadProgress (qint64, qint64)));
	}

	QUrl DriveManager::GenerateDownloadUrl (const QString& id) const
	{
		return QUrl (QString ("https://api-content.dropbox.com/1/files/%1/%2?access_token=%3")
				.arg ("dropbox")
				.arg (id)
				.arg (Account_->GetAccessToken ()));
	}

	void DriveManager::ParseError (const QVariantMap& map)
	{
		QString msg = map ["error"].toString ();
		Core::Instance ().SendEntity (Util::MakeNotification ("NetStoreManager",
				msg,
				Priority::Warning));
	}

	void DriveManager::handleGotAccountInfo ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;
		reply->deleteLater ();

		const auto& res = Util::ParseJson (reply, Q_FUNC_INFO);
		if (res.isNull ())
			return;

		Account_->SetUserID (res.toMap () ["uid"].toString ());
	}

	void DriveManager::handleCreateDirectory ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;
		reply->deleteLater ();

		const auto& res = Util::ParseJson (reply, Q_FUNC_INFO);
		if (res.isNull ())
			return;

		qDebug () << Q_FUNC_INFO
				<< "directory created successfully";
		emit gotNewItem (CreateDBoxItem (res.toMap ()));
	}

	void DriveManager::handleRequestEntryRemoving ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		const auto& res = Util::ParseJson (reply, Q_FUNC_INFO);
		if (res.isNull ())
			return;

		qDebug () << Q_FUNC_INFO
				<< "file removed successfully";
		emit gotNewItem (CreateDBoxItem (res));
	}

	void DriveManager::handleCopyItem ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;
		reply->deleteLater ();

		const auto& res = Util::ParseJson (reply, Q_FUNC_INFO);
		if (res.isNull ())
			return;

		qDebug () << Q_FUNC_INFO
				<< "entry copied successfully";
		RefreshListing (Reply2Id_.take (reply).toUtf8 ());
	}

	void DriveManager::handleMoveItem ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;
		reply->deleteLater ();

		const auto& res = Util::ParseJson (reply, Q_FUNC_INFO);
		if (res.isNull ())
			return;

		qDebug () << Q_FUNC_INFO
				<< "entry moved successfully";
		RefreshListing (Reply2Id_.take (reply).toUtf8 ());
	}

	void DriveManager::handleUploadFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;
		reply->deleteLater ();

		const auto& res = Util::ParseJson (reply, Q_FUNC_INFO);
		if (res.isNull ())
			return;

		const auto& map = res.toMap ();
		const auto& id = map ["id"].toString ();

		if (map.contains ("error"))
		{
			ParseError (map);
			return;
		}

		qDebug () << Q_FUNC_INFO
				<< "file uploaded successfully";
		emit gotNewItem (CreateDBoxItem (res));
		emit finished (id, Reply2FilePath_.take (reply));

	}

	void DriveManager::handleChunkUploadFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;
		reply->deleteLater ();

		const auto& res = Util::ParseJson (reply, Q_FUNC_INFO);
		if (res.isNull ())
			return;

		const auto& map = res.toMap ();
		if (!map.contains ("error"))
		{
			if (map.contains ("offset"))
			{
				const quint64 offset = map ["offset"].toULongLong ();
				const QString uploadId = map ["upload_id"].toString ();
				RequestChunkUpload (Reply2FilePath_.take (reply),
						Reply2ParentId_.take (reply),
						uploadId,
						offset);
			}
			else if (map.contains ("path"))
			{
				qDebug () << Q_FUNC_INFO
						<< "file uploaded successfully";
				emit gotNewItem (CreateDBoxItem (res));
				emit finished (Reply2Id_.take (reply), Reply2FilePath_.take (reply));
			}
			return;
		}

		ParseError (map);
	}

	void DriveManager::handleUploadProgress (qint64 uploaded, qint64 total)
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		const auto& path = Reply2FilePath_ [reply];
		const quint64 offset = Reply2Offset_ [reply];
		QFileInfo fi (path);
		if (fi.size () < ChunkUploadBound)
			emit uploadProgress (uploaded, total, path);
		else
			emit uploadProgress (uploaded + offset, fi.size (), path);
	}

	void DriveManager::handleUploadError (QNetworkReply::NetworkError)
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;
		reply->deleteLater ();

		emit uploadError ("Error", Reply2FilePath_.take (reply));
	}
}
}
}
