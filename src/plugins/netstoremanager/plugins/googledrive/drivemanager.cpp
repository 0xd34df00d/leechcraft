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
#include <QNetworkReply>
#include <QtDebug>
#include <qjson/parser.h>
#include <qjson/serializer.h>
#include "account.h"
#include "core.h"

namespace LeechCraft
{
namespace NetStoreManager
{
namespace GoogleDrive
{
	DriveManager::DriveManager (Account *acc, QObject *parent)
	: QObject (parent)
	, Account_ (acc)
	{
	}

	void DriveManager::RefreshListing ()
	{
		ApiCallQueue_ << [this] (const QString& key) { RequestFiles (key); };
		RequestAccessToken ();
	}

	void DriveManager::RemoveEntry (const QString& id)
	{
		ApiCallQueue_ << [this, id] (const QString& key) { RequestEntryRemoving (id, key); };
		RequestAccessToken ();
	}

	void DriveManager::MoveEntryToTrash (const QString& id)
	{
		ApiCallQueue_ << [this, id] (const QString& key) { RequestMovingEntryToTrash (id, key); };
		RequestAccessToken ();
	}

	void DriveManager::RequestFiles (const QString& key)
	{
		QString str = QString ("https://www.googleapis.com/drive/v2/files?access_token=%1")
				.arg (key);
		QNetworkRequest request (str);

		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

		QNetworkReply *reply = Core::Instance ().GetProxy ()->
				GetNetworkAccessManager ()->get (request);

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotFiles ()));
	}

	void DriveManager::RequestFileShared (const QString& id, const QString& key)
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

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleRequestFileSharing ()));
	}

	void DriveManager::RequestEntryRemoving (const QString& id, const QString& key)
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

	void DriveManager::RequestMovingEntryToTrash (const QString& id, const QString& key)
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

	void DriveManager::ParseError (const QVariantMap& map)
	{
		const auto& errorMap = map ["error"].toMap ();
		const QString& code = errorMap ["code"].toString ();
		const QString& msg = errorMap ["message"].toString ();

		//TODO error notification
	}

	void DriveManager::handleAuthTokenRequestFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		bool ok = false;
		QByteArray ba = reply->readAll ();
		QVariant res = QJson::Parser ().parse (ba, &ok);

		if (!ok)
		{
			qDebug () << Q_FUNC_INFO << "parse error";
			return;
		}

		QString accessKey = res.toMap ().value ("access_token").toString ();
		if (accessKey.isEmpty ())
		{
			qDebug () << Q_FUNC_INFO << "access token is empty";
			return;
		}

		if (ApiCallQueue_.isEmpty ())
			return;

		ApiCallQueue_.dequeue () (accessKey);
	}

	void DriveManager::handleGotFiles ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		bool ok = false;
		QByteArray ba = reply->readAll ();
		QVariant res = QJson::Parser ().parse (ba, &ok);

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
			QVariantMap map = item.toMap ();
			if (map ["mimeType"].toString () != "application/vnd.google-apps.folder" &&
					map ["downloadUrl"].toString ().isEmpty ())
				continue;

			DriveItem driveItem;
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

			const QVariantMap& permission = map ["userPermission"].toMap ();
			const QString& role = permission ["role"].toString ();
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

			driveItem.OriginalFileName_ = map ["originalFilename"].toString ();
			driveItem.Md5_ = map ["md5Checksum"].toString ();
			driveItem.FileSize_ = map ["fileSize"].toLongLong ();

			for (const auto& ownerName : map ["ownerNames"].toList ())
				driveItem.OwnerNames_ << ownerName.toString ();

			driveItem.LastModifiedBy_ = map ["lastModifyingUserName"].toString ();
			driveItem.Editable_ = map ["editable"].toBool ();
			driveItem.WritersCanShare_ = map ["writersCanShare"].toBool ();

			const auto& parent = map ["parents"].toList ().value (0).toMap ();
			driveItem.ParentId_ = parent ["id"].toString ();
			driveItem.ParentIsRoot_ = parent ["isRoot"].toBool ();

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
		QByteArray ba = reply->readAll ();
		QVariant res = QJson::Parser ().parse (ba, &ok);

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
		QByteArray ba = reply->readAll ();
		QVariant res = QJson::Parser ().parse (ba, &ok);
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
			qDebug () << Q_FUNC_INFO << res.toMap ();
			RefreshListing ();
			return;
		}
	}

	void DriveManager::handleRequestMovingEntryToTrash ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		bool ok = false;
		QByteArray ba = reply->readAll ();
		QVariant res = QJson::Parser ().parse (ba, &ok);
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
			qDebug () << Q_FUNC_INFO << res.toMap ();
			RefreshListing ();
			return;
		}

	}

}
}
}

