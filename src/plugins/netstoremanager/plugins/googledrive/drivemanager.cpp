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

#include "drivemanager.h"
#include <QNetworkRequest>
#include <QtDebug>
#include <QFileInfo>
#include <QDesktopServices>
#include <util/util.h>
#include <qjson/parser.h>
#include <qjson/serializer.h>
#include "account.h"
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace NetStoreManager
{
namespace GoogleDrive
{
	DriveManager::DriveManager (Account *acc, QObject *parent)
	: QObject (parent)
	, DirectoryId_ ("application/vnd.google-apps.folder")
	, Account_ (acc)
	{
#ifdef HAVE_MAGIC
		Magic_ = magic_open (MAGIC_MIME_TYPE);
		magic_load (Magic_, NULL);
#endif
	}

	DriveManager::~DriveManager ()
	{
#ifdef HAVE_MAGIC
		magic_close (Magic_);
#endif
	}

	void DriveManager::RefreshListing ()
	{
		ApiCallQueue_ << [this] (const QString& key) { RequestFiles (key); };
		RequestAccessToken ();
	}

	void DriveManager::RemoveEntry (const QString& id)
	{
		if (id.isEmpty ())
			return;
		ApiCallQueue_ << [this, id] (const QString& key) { RequestEntryRemoving (id, key); };
		RequestAccessToken ();
	}

	void DriveManager::MoveEntryToTrash (const QString& id)
	{
		if (id.isEmpty ())
			return;
		ApiCallQueue_ << [this, id] (const QString& key) { RequestMovingEntryToTrash (id, key); };
		RequestAccessToken ();
	}

	void DriveManager::RestoreEntryFromTrash (const QString& id)
	{
		if (id.isEmpty ())
			return;
		ApiCallQueue_ << [this, id] (const QString& key) { RequestRestoreEntryFromTrash (id, key); };
		RequestAccessToken ();
	}

	void DriveManager::ShareEntry (const QString& id)
	{
		if (id.isEmpty ())
			return;
		ApiCallQueue_ << [this, id] (const QString& key) { RequestSharingEntry (id, key); };
		RequestAccessToken ();
	}

	void DriveManager::Upload (const QString& filePath, const QStringList& parentId)
	{
		QString parent = parentId.value (0);
		ApiCallQueue_ << [this, filePath, parent] (const QString& key) { RequestUpload (filePath, parent, key); };
		RequestAccessToken ();
	}

	void DriveManager::Download (const QString& id, const QString& filepath,
			bool silent)
	{
		if (id.isEmpty ())
			return;
		ApiCallQueue_ << [this, id] (const QString& key) { RequestFileInfo (id, key); };
		DownloadsQueue_ << [this, filepath, silent] (const QUrl& url) { DownloadFile (filepath, url, silent); };
		RequestAccessToken ();
	}

	void DriveManager::CreateDirectory (const QString& name,
			const QString& parentId)
	{
		ApiCallQueue_ << [this, name, parentId] (const QString& key)
			{ RequestCreateDirectory (name, parentId, key); };
		RequestAccessToken ();
	}

	void DriveManager::Copy (const QString& id, const QString& parentId)
	{
		if (id.isEmpty ())
			return;
		ApiCallQueue_ << [this, id, parentId] (const QString& key) { RequestCopyItem (id, parentId, key); };
		RequestAccessToken ();
	}

	void DriveManager::Move (const QString& id, const QString& parentId)
	{
		if (id.isEmpty ())
			return;
		ApiCallQueue_ << [this, id, parentId] (const QString& key) { RequestMoveItem (id, parentId, key); };
		RequestAccessToken ();
	}

	void DriveManager::Rename (const QString& id, const QString& newName)
	{
		if (id.isEmpty ())
			return;
		ApiCallQueue_ << [this, id, newName] (const QString& key) { RequestRenameItem (id, newName, key); };
		RequestAccessToken ();
	}

	void DriveManager::RequestFileChanges (qlonglong startId)
	{
		ApiCallQueue_ << [this, startId] (const QString& key) { GetFileChanges (startId, key); };
		RequestAccessToken ();
	}

	void DriveManager::RequestFiles (const QString& key)
	{
		QString str = QString ("https://www.googleapis.com/drive/v2/files?access_token=%1")
				.arg (key);
		QNetworkRequest request (str);

		request.setHeader (QNetworkRequest::ContentTypeHeader,
				"application/x-www-form-urlencoded");

		QNetworkReply *reply = Core::Instance ().GetProxy ()->
				GetNetworkAccessManager ()->get (request);

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotFiles ()));
	}

	void DriveManager::RequestSharingEntry (const QString& id,
			const QString& key)
	{
		QString str = QString ("https://www.googleapis.com/drive/v2/files/%1/permissions?access_token=%2")
				.arg (id, key);
		QNetworkRequest request (str);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");

		QVariantMap map;
		map.insert ("kind", "drive#permission");
		map.insert ("id", "anyoneWithLink");
		map.insert ("role", "reader");
		map.insert ("type", "anyone");
		map.insert ("withLink", true);

		QJson::Serializer serializer;

		QNetworkReply *reply = Core::Instance ().GetProxy ()->
				GetNetworkAccessManager ()->post (request, serializer.serialize (map));
		Reply2Id_ [reply] = id;

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleRequestFileSharing ()));
	}

	void DriveManager::RequestEntryRemoving (const QString& id,
			const QString& key)
	{
		QString str = QString ("https://www.googleapis.com/drive/v2/files/%1?access_token=%2")
				.arg (id, key);
		QNetworkRequest request (str);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");

		QNetworkReply *reply = Core::Instance ().GetProxy ()->
				GetNetworkAccessManager ()->deleteResource (request);

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleRequestEntryRemoving ()));
	}

	void DriveManager::RequestMovingEntryToTrash (const QString& id,
			const QString& key)
	{
		QString str = QString ("https://www.googleapis.com/drive/v2/files/%1/trash?access_token=%2")
				.arg (id, key);
		QNetworkRequest request (str);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");

		QNetworkReply *reply = Core::Instance ().GetProxy ()->
				GetNetworkAccessManager ()->post (request, QByteArray ());

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleRequestMovingEntryToTrash ()));
	}

	void DriveManager::RequestRestoreEntryFromTrash (const QString& id,
			const QString& key)
	{
		QString str = QString ("https://www.googleapis.com/drive/v2/files/%1/untrash?access_token=%2")
				.arg (id, key);
		QNetworkRequest request (str);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");

		QNetworkReply *reply = Core::Instance ().GetProxy ()->
				GetNetworkAccessManager ()->post (request, QByteArray ());

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleRequestRestoreEntryFromTrash ()));
	}

	void DriveManager::RequestAccessToken ()
	{
		QNetworkRequest request (QUrl ("https://accounts.google.com/o/oauth2/token"));
		QString str = QString ("refresh_token=%1&client_id=%2&client_secret=%3&grant_type=%4")
				.arg (Account_->GetRefreshToken ())
				.arg ("844868161425.apps.googleusercontent.com")
				.arg ("l09HkM6nbPMEYcMdcdeGBdaV")
				.arg ("refresh_token");

		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

		QNetworkReply *reply = Core::Instance ().GetProxy ()->
				GetNetworkAccessManager ()->post (request, str.toUtf8 ());

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleAuthTokenRequestFinished ()));
	}

	void DriveManager::RequestUpload (const QString& filePath,
			const QString& parent, const QString& key)
	{
		emit uploadStatusChanged (tr ("Initializing..."), filePath);

		QFileInfo info (filePath);
		const QUrl initiateUrl (QString ("https://www.googleapis.com/upload/drive/v2/files?access_token=%1&uploadType=resumable")
				.arg (key));
		QNetworkRequest request (initiateUrl);
		request.setPriority (QNetworkRequest::LowPriority);
#ifdef HAVE_MAGIC
		request.setRawHeader ("X-Upload-Content-Type",
				magic_file (Magic_, filePath.toUtf8 ()));
#endif
		request.setRawHeader ("X-Upload-Content-Length",
				QString::number (QFileInfo (filePath).size ()).toUtf8 ());
		QVariantMap map;
		map ["title"] = QFileInfo (filePath).fileName ();
		if (!parent.isEmpty ())
		{
			QVariantList parents;
			QVariantMap parentMap;
			parentMap ["id"] = parent;
			parents << parentMap;
			map ["parents"] = parents;
		}

		const auto& data = QJson::Serializer ().serialize (map);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");
		request.setHeader (QNetworkRequest::ContentLengthHeader, data.size ());

		QNetworkReply *reply = Core::Instance ().GetProxy ()->
				GetNetworkAccessManager ()->post (request, data);
		Reply2FilePath_ [reply] = filePath;

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleUploadRequestFinished ()));
	}

	void DriveManager::RequestCreateDirectory (const QString& name,
			const QString& parentId, const QString& key)
	{
		QString str = QString ("https://www.googleapis.com/drive/v2/files?access_token=%1")
				.arg (key);

		QNetworkRequest request (str);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");
		QVariantMap data;
		data ["title"] = name;
		data ["mimeType"] = DirectoryId_;
		if (!parentId.isEmpty ())
		{
			QVariantList parents;
			QVariantMap parent;
			parent ["id"] = parentId;
			parents << parent;
			data ["parents"] = parents;
		}

		QNetworkReply *reply = Core::Instance ().GetProxy ()->GetNetworkAccessManager ()->
				post (request, QJson::Serializer ().serialize (data));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleCreateDirectory ()));
	}

	void DriveManager::RequestCopyItem (const QString& id,
			const QString& parentId, const QString& key)
	{
		QString str = QString ("https://www.googleapis.com/drive/v2/files/%1/copy?access_token=%2")
				.arg (id)
				.arg (key);

		QNetworkRequest request (str);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");
		QVariantMap data;
		if (!parentId.isEmpty ())
		{
			QVariantList parents;
			QVariantMap parent;
			parent ["id"] = parentId;
			parents << parent;
			data ["parents"] = parents;
		}

		QNetworkReply *reply = Core::Instance ().GetProxy ()->GetNetworkAccessManager ()->
				post (request, QJson::Serializer ().serialize (data));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleCopyItem ()));
	}

	void DriveManager::RequestMoveItem (const QString& id,
			const QString& parentId, const QString& key)
	{
		QString str = QString ("https://www.googleapis.com/drive/v2/files/%1?access_token=%2")
				.arg (id)
				.arg (key);

		QNetworkRequest request (str);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");
		QVariantMap data;
		if (!parentId.isEmpty ())
		{
			QVariantList parents;
			QVariantMap parent;
			parent ["id"] = parentId;
			parents << parent;
			data ["parents"] = parents;
		}

		QNetworkReply *reply = Core::Instance ().GetProxy ()->GetNetworkAccessManager ()->
				put (request, QJson::Serializer ().serialize (data));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleMoveItem ()));
	}

	void DriveManager::GetFileChanges (qlonglong startId, const QString& key)
	{
		const QString str = startId ?
			QString ("https://www.googleapis.com/drive/v2/changes?includeDeleted=true&startChangeId=%1&access_token=%2")
					.arg (startId)
					.arg (key) :
			QString ("https://www.googleapis.com/drive/v2/changes?includeDeleted=true&access_token=%1")
					.arg (key);

		QNetworkReply *reply = Core::Instance ().GetProxy ()->GetNetworkAccessManager ()->
				get (QNetworkRequest (str));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGetFileChanges ()));
	}

	void DriveManager::RequestFileInfo (const QString& id, const QString& key)
	{
		QString str = QString ("https://www.googleapis.com/drive/v2/files/%1?access_token=%2")
				.arg (id)
				.arg (key);

		QNetworkReply *reply = Core::Instance ().GetProxy ()->GetNetworkAccessManager ()->
				get (QNetworkRequest (str));
		Reply2DownloadAccessToken_ [reply] = key;
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGetFileInfo ()));
	}

	void DriveManager::RequestRenameItem (const QString& id, const QString& name, const QString& key)
	{
		QString str = QString ("https://www.googleapis.com/drive/v2/files/%1?access_token=%2")
				.arg (id)
				.arg (key);

		QNetworkRequest request (str);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");
		QVariantMap data;
		data ["title"] = name;
		QNetworkReply *reply = Core::Instance ().GetProxy ()->GetNetworkAccessManager ()->
				put (request, QJson::Serializer ().serialize (data));
		Reply2DownloadAccessToken_ [reply] = key;
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleItemRenamed ()));
	}

	void DriveManager::DownloadFile (const QString& filePath, const QUrl& url,
			bool silent)
	{
		TaskParameters tp = OnlyDownload | FromUserInitiated;
		if (silent)
			tp |= AutoAccept | Internal |
					DoNotNotifyUser |
					DoNotSaveInHistory |
					DoNotAnnounceEntity;
		LeechCraft::Entity e = Util::MakeEntity (url,
				QDesktopServices::storageLocation (QDesktopServices::TempLocation) +
						"/" + QFileInfo (filePath).fileName (),
				tp);
		silent ?
			Core::Instance ().DelegateEntity (e, filePath) :
			Core::Instance ().SendEntity (e);
	}

	void DriveManager::FindSyncableItems (const QStringList& paths,
			const QString& baseDir, const QList<DriveItem>& items)
	{
		const QString& baseName = QFileInfo (baseDir).fileName ();

		DriveItem rootItem;
		bool found = false;
		for (const auto& item : items)
			if (item.Name_ == baseName &&
					item.IsFolder_ &&
					!(item.Labels_ & DriveItem::ILRemoved))
			{
				rootItem = item;
				found = true;
				break;
			}

		if (!found)
			CreateDirectory (baseName);
	}

	void DriveManager::ParseError (const QVariantMap& map)
	{
		const auto& errorMap = map ["error"].toMap ();
		const QString& code = errorMap ["code"].toString ();
		QString msg = errorMap ["message"].toString ();

		//TODO fix false execute
// 		if (code == "500")
// 			msg = tr ("Google Drive API v.2 doesn't support directory copying.");
		Core::Instance ().SendEntity (Util::MakeNotification ("NetStoreManager",
				msg,
				PWarning_));
	}

	void DriveManager::handleAuthTokenRequestFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		bool ok = false;
		QVariant res = QJson::Parser ().parse (reply->readAll (), &ok);

		if (!ok)
		{
			qDebug () << Q_FUNC_INFO << "parse error";
			return;
		}

		QString accessKey = res.toMap ().value ("access_token").toString ();
		qDebug () << accessKey;
		if (accessKey.isEmpty ())
		{
			qDebug () << Q_FUNC_INFO << "access token is empty";
			return;
		}

		if (ApiCallQueue_.isEmpty ())
			return;

		ApiCallQueue_.dequeue () (accessKey);
	}

	namespace
	{
		DriveItem CreateDriveItem (const QVariant& itemData)
		{
			QVariantMap map = itemData.toMap ();

			const QVariantMap& permission = map ["userPermission"].toMap ();
			const QString& role = permission ["role"].toString ();

			if (role != "owner")
				return DriveItem ();

			DriveItem driveItem;

			const QString& type = permission ["type"].toString ();

			driveItem.PermissionAdditionalRole_ = DriveItem::ARNone;
			if (permission ["additionalRoles"].toList ().contains ("commenter"))
				driveItem.PermissionAdditionalRole_ |= DriveItem::ARCommenter;

			if (role == "owner")
				driveItem.PermissionRole_ = DriveItem::Roles::Owner;
			else if (role == "writer")
				driveItem.PermissionRole_ = DriveItem::Roles::Writer;
			else if (role == "reader")
				driveItem.PermissionRole_ = DriveItem::Roles::Reader;

			if (type == "user")
				driveItem.PermissionType_ = DriveItem::PermissionTypes::User;

			driveItem.Id_ = map ["id"].toString ();
			driveItem.Name_ = map ["title"].toString ();
			driveItem.IsFolder_ = map ["mimeType"].toString () ==
					"application/vnd.google-apps.folder";
			driveItem.Mime_ = map ["mimeType"].toString ();

			driveItem.DownloadUrl_ = map ["downloadUrl"].toUrl ();

			const QVariantMap& labels = map ["labels"].toMap ();
			driveItem.Labels_ = DriveItem::ILNone;
			if (labels ["starred"].toBool ())
				driveItem.Labels_ |= DriveItem::ILFavorite;
			if (labels ["hidden"].toBool ())
				driveItem.Labels_ |= DriveItem::ILHidden;
			if (labels ["trashed"].toBool ())
				driveItem.Labels_ |= DriveItem::ILRemoved;
			if (labels ["restricted"].toBool ())
				driveItem.Labels_ |= DriveItem::ILShared;
			if (labels ["viewed"].toBool ())
				driveItem.Labels_ |= DriveItem::ILViewed;
			driveItem.CreateDate_ = QDateTime::fromString (map ["createdDate"].toString (),
					Qt::ISODate);
			driveItem.ModifiedDate_ = QDateTime::fromString (map ["modifiedDate"].toString (),
					Qt::ISODate);
			driveItem.LastViewedByMe_ = QDateTime::fromString (map ["lastViewedByMeDate"].toString (),
					Qt::ISODate);

			driveItem.OriginalFileName_ = map ["originalFilename"].toString ();
			driveItem.Md5_ = map ["md5Checksum"].toString ();
			driveItem.FileSize_ = map ["fileSize"].toLongLong ();

			for (const auto& ownerName : map ["ownerNames"].toList ())
				driveItem.OwnerNames_ << ownerName.toString ();

			driveItem.LastModifiedBy_ = map ["lastModifyingUserName"].toString ();
			driveItem.Editable_ = map ["editable"].toBool ();
			driveItem.WritersCanShare_ = map ["writersCanShare"].toBool ();

			const auto& parent = map ["parents"].toList ().value (0).toMap ();
			if (!parent.isEmpty ())
			{
				driveItem.ParentId_ = parent ["id"].toString ();
				driveItem.ParentIsRoot_ = parent ["isRoot"].toBool ();
			}

			return driveItem;
		}
	}

	void DriveManager::handleGotFiles ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		bool ok = false;
		const auto& res = QJson::Parser ().parse (reply->readAll (), &ok);

		if (!ok)
		{
			qDebug () << Q_FUNC_INFO << "parse error";
			return;
		}

		const auto& resMap = res.toMap ();
		if (!resMap.contains ("items"))
		{
			qDebug () << Q_FUNC_INFO << "there are no items";
			return;
		}

		if (resMap.contains ("error"))
		{
			ParseError (res.toMap ());
			return;
		}

		QList<DriveItem> resList;
		Q_FOREACH (const auto& item, resMap ["items"].toList ())
		{
			const auto& driveItem = CreateDriveItem (item);
			if (driveItem.Name_.isEmpty ())
				continue;
			resList << driveItem;
		}

		emit gotFiles (resList);
	}

	void DriveManager::handleRequestFileSharing ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		bool ok = false;
		const auto& res = QJson::Parser ().parse (reply->readAll (), &ok);

		if (!ok)
		{
			qDebug () << Q_FUNC_INFO
					<< "parse error";
			return;
		}

		if (!res.toMap ().contains ("error"))
		{
			qDebug () << Q_FUNC_INFO
					<< "file shared successfully";
			emit gotSharedFileId (Reply2Id_.take (reply));
			return;
		}

		ParseError (res.toMap ());
	}

	void DriveManager::handleRequestEntryRemoving ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		bool ok = false;
		const auto& res = QJson::Parser ().parse (reply->readAll (), &ok);
		if (!ok)
		{
			qDebug () << Q_FUNC_INFO
					<< "parse error";
			return;
		}

		if (!res.toMap ().contains ("error"))
		{
			qDebug () << Q_FUNC_INFO
					<< "file removed successfully";
			RefreshListing ();
			return;
		}

		ParseError (res.toMap ());
	}

	void DriveManager::handleRequestMovingEntryToTrash ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		bool ok = false;
		const auto& res = QJson::Parser ().parse (reply->readAll (), &ok);
		if (!ok)
		{
			qDebug () << Q_FUNC_INFO
					<< "parse error";
			return;
		}

		if (!res.toMap ().contains ("error"))
		{
			qDebug () << Q_FUNC_INFO
					<< "file moved to trash successfully";
			RefreshListing ();
			return;
		}

		ParseError (res.toMap ());
	}

	void DriveManager::handleRequestRestoreEntryFromTrash ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		bool ok = false;
		const auto& res = QJson::Parser ().parse (reply->readAll (), &ok);
		if (!ok)
		{
			qDebug () << Q_FUNC_INFO
					<< "parse error";
			return;
		}

		if (!res.toMap ().contains ("error"))
		{
			qDebug () << Q_FUNC_INFO
					<< "file restored from trash successfully";
			RefreshListing ();
			return;
		}

		ParseError (res.toMap ());
	}

	void DriveManager::handleUploadRequestFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();
		QString path = Reply2FilePath_.take (reply);

		const int code = reply->
				attribute (QNetworkRequest::HttpStatusCodeAttribute).toInt ();
		if (code != 200)
		{
			qWarning () << Q_FUNC_INFO
					<< "upload initiating failed with code:"
					<< code;
			return;
		}

		emit uploadStatusChanged (tr ("Uploading..."), path);

		QFile *file = new QFile (path);
		if (!file->open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file: "
					<< file->errorString ();
			return;
		}

		QUrl url (reply->rawHeader ("Location"));
		QNetworkRequest request (url);
#ifdef HAVE_MAGIC
		request.setHeader (QNetworkRequest::ContentTypeHeader,
				magic_file (Magic_, path.toUtf8 ()));
#endif
		request.setHeader (QNetworkRequest::ContentLengthHeader,
				QString::number (QFileInfo (path).size ()).toUtf8 ());

		QNetworkReply *uploadReply = Core::Instance ().GetProxy ()->
				GetNetworkAccessManager ()->put (request, file);
		file->setParent (uploadReply);
		Reply2FilePath_ [uploadReply] = path;

		connect (uploadReply,
				SIGNAL (finished ()),
				this,
				SLOT (handleUploadFinished ()));
		connect (uploadReply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleUploadError (QNetworkReply::NetworkError)));
		connect (uploadReply,
				SIGNAL (uploadProgress (qint64, qint64)),
				this,
				SLOT (handleUploadProgress (qint64, qint64)));
	}

	void DriveManager::handleUploadFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		bool ok = false;
		const auto& res = QJson::Parser ().parse (reply->readAll (), &ok);
		if (!ok)
		{
			qDebug () << Q_FUNC_INFO
					<< "parse error";
			return;
		}

		const auto& map = res.toMap ();
		const auto& id = map ["id"].toString ();

		if (!map.contains ("error"))
		{
			qDebug () << Q_FUNC_INFO
					<< "file uploaded successfully";
			RefreshListing ();
			emit gotNewItem (CreateDriveItem (res));
			emit finished (id, Reply2FilePath_.take (reply));
			return;
		}

		 ParseError (map);
	}

	void DriveManager::handleUploadProgress (qint64 uploaded, qint64 total)
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		emit uploadProgress (uploaded, total, Reply2FilePath_ [reply]);
	}

	void DriveManager::handleUploadError (QNetworkReply::NetworkError error)
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		emit uploadError ("Error", Reply2FilePath_.take (reply));
		if (error == QNetworkReply::ProtocolUnknownError)
		{
			//TODO resume upload
		}
	}

	void DriveManager::handleCreateDirectory ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		bool ok = false;
		const auto& res = QJson::Parser ().parse (reply->readAll (), &ok);
		if (!ok)
		{
			qDebug () << Q_FUNC_INFO
					<< "parse error";
			return;
		}

		if (!res.toMap ().contains ("error"))
		{
			qDebug () << Q_FUNC_INFO
					<< "directory created successfully";

			emit gotNewItem (CreateDriveItem (res));
			return;
		}

		ParseError (res.toMap ());
	}

	void DriveManager::handleCopyItem ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		bool ok = false;
		const auto& res = QJson::Parser ().parse (reply->readAll (), &ok);
		if (!ok)
		{
			qDebug () << Q_FUNC_INFO
					<< "parse error";
			return;
		}

		if (!res.toMap ().contains ("error"))
		{
			qDebug () << Q_FUNC_INFO
					<< "entry copied successfully";
			RefreshListing ();
			return;
		}

		ParseError (res.toMap ());
	}

	void DriveManager::handleMoveItem ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		bool ok = false;
		const auto& res = QJson::Parser ().parse (reply->readAll (), &ok);
		if (!ok)
		{
			qDebug () << Q_FUNC_INFO
					<< "parse error";
			return;
		}

		if (!res.toMap ().contains ("error"))
		{
			qDebug () << Q_FUNC_INFO
					<< "entry moved successfully";
			RefreshListing ();
			return;
		}

		ParseError (res.toMap ());
	}

	void DriveManager::handleGetFileChanges ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		bool ok = false;
		const auto& res = QJson::Parser ().parse (reply->readAll (), &ok);
		if (!ok)
		{
			qDebug () << Q_FUNC_INFO
					<< "parse error";
			return;
		}

		const QVariantMap& map = res.toMap ();
		if (map.contains ("error"))
		{
			ParseError (map);
			return;
		}

		QList<DriveChanges> changes;

		if (!map.contains ("items") ||
				map ["items"].toList ().isEmpty ())
			return;

		for (auto itemVar : map ["items"].toList ())
		{
			QVariantMap itemMap = itemVar.toMap ();
			DriveChanges change;
			DriveItem item = CreateDriveItem (itemMap ["file"]);
			change.FileId_ = itemMap ["fileId"].toString ();
			change.Id_ = itemMap ["id"].toString ();
			change.Deleted_ = itemMap ["deleted"].toBool ();
			change.FileResource_ = item;

			changes << change;
		}

		gotChanges (changes, map ["largestChangeId"].toLongLong () + 1);
	}

	void DriveManager::handleGetFileInfo ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		bool ok = false;
		const auto& res = QJson::Parser ().parse (reply->readAll (), &ok);
		if (!ok)
		{
			qDebug () << Q_FUNC_INFO
					<< "parse error";
			return;
		}

		const QVariantMap& map = res.toMap ();
		QString access_token = Reply2DownloadAccessToken_.take (reply);

		if (!map.contains ("error"))
		{
			DriveItem it = CreateDriveItem (res);
			if (!access_token.isEmpty ())
				it.DownloadUrl_.addQueryItem ("access_token", access_token);

			if (!DownloadsQueue_.isEmpty ())
				DownloadsQueue_.dequeue () (it.DownloadUrl_);
			return;
		}

		ParseError (map);
	}

	void DriveManager::handleItemRenamed ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		bool ok = false;
		const auto& res = QJson::Parser ().parse (reply->readAll (), &ok);
		if (!ok)
		{
			qDebug () << Q_FUNC_INFO
					<< "parse error";
			return;
		}

		const QVariantMap& map = res.toMap ();

		if (!map.contains ("error"))
		{
			DriveItem it = CreateDriveItem (res);
			qDebug () << Q_FUNC_INFO
					<< "entry renamed successfully";
			emit gotNewItem (it);

			return;
		}

		ParseError (map);
	}

}
}
}

