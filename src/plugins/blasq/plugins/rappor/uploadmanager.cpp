/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "uploadmanager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QFileInfo>
#include <QFile>
#include <QDomDocument>
#include <QStringList>
#include <QtDebug>
#include <util/sll/parsejson.h>
#include <util/sll/queuemanager.h>
#include <util/sll/urloperator.h>
#include "vkaccount.h"

namespace LC
{
namespace Blasq
{
namespace Rappor
{
	UploadManager::UploadManager (Util::QueueManager *reqQueue, ICoreProxy_ptr proxy, VkAccount *acc)
	: QObject (acc)
	, Acc_ (acc)
	, Proxy_ (proxy)
	, RequestQueue_ (reqQueue)
	{
	}

	void UploadManager::Upload (const QString& aidStr, const QList<UploadItem>& items)
	{
		Acc_->Schedule ([this, items, aidStr] (const QString& authKey)
			{
				QUrl getUrl ("https://api.vk.com/method/photos.getUploadServer.xml");
				Util::UrlOperator { getUrl }
						("aid", aidStr)
						("access_token", authKey);
				RequestQueue_->Schedule ([this, getUrl, items]
					{
						auto reply = Proxy_->GetNetworkAccessManager ()->
								get (QNetworkRequest (getUrl));
						connect (reply,
								SIGNAL (finished ()),
								this,
								SLOT (handlePhotosUploadServer ()));
						PhotosUploadServer2Infos_ [reply] = items;
					}, this);
			});
	}

	void UploadManager::StartUpload (const QString& server, QList<UploadItem> infos)
	{
		if (infos.isEmpty ())
			return;

		const auto& info = infos.takeFirst ();

		const auto& path = info.FilePath_;

		auto file = new QFile (path);
		if (!file->open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "error opening file:"
					<< path
					<< file->errorString ();

			StartUpload (server, infos);

			delete file;

			return;
		}

		auto multipart = new QHttpMultiPart (QHttpMultiPart::FormDataType);
		file->setParent (multipart);

		QHttpPart filePart;

		const auto& disp = QString ("form-data; name=\"file1\"; filename=\"%1\"")
				.arg (QFileInfo (path).fileName ());
		filePart.setHeader (QNetworkRequest::ContentDispositionHeader, disp);

		filePart.setBodyDevice (file);

		multipart->append (filePart);

		const auto nam = Proxy_->GetNetworkAccessManager ();
		auto reply = nam->post (QNetworkRequest (QUrl (server)), multipart);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handlePhotosUploaded ()));
		connect (reply,
				SIGNAL (uploadProgress (qint64, qint64)),
				this,
				SLOT (handlePhotosUploadProgress (qint64, qint64)));
		PhotoUpload2Info_ [reply] = info;
		PhotoUpload2QueueTail_ [reply] = infos;
		PhotoUpload2Server_ [reply] = server;
		multipart->setParent (reply);
	}

	void UploadManager::handlePhotosUploadServer ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = reply->readAll ();
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot parse reply"
					<< data;
			return;
		}

		const auto& server = doc.documentElement ().firstChildElement ("upload_url").text ();
		StartUpload (server, PhotosUploadServer2Infos_.take (reply));
	}

	void UploadManager::handlePhotosUploadProgress (qint64 done, qint64 total)
	{
		qDebug () << "upload" << done << total;
	}

	void UploadManager::handlePhotosUploaded ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& tail = PhotoUpload2QueueTail_.take (reply);
		const auto& server = PhotoUpload2Server_.take (reply);
		StartUpload (server, tail);

		const auto& data = reply->readAll ();
		const auto& parsed = Util::ParseJson (data, Q_FUNC_INFO).toMap ();

		const auto& info = PhotoUpload2Info_.take (reply);

		Acc_->Schedule ([this, parsed, info] (const QString& authKey)
			{
				QUrl saveUrl ("https://api.vk.com/method/photos.save.xml");
				{
					Util::UrlOperator op { saveUrl };
					auto add = [&parsed, &op] (const QString& name)
						{ op (name, parsed [name].toString ()); };
					add ("server");
					add ("photos_list");
					add ("aid");
					add ("hash");
					op ("access_token", authKey);

					if (!info.Description_.isEmpty ())
						op ("caption", info.Description_);
				}

				RequestQueue_->Schedule ([this, saveUrl, info]
					{
						const auto req = Proxy_->GetNetworkAccessManager ()->get (QNetworkRequest (saveUrl));
						PhotoSave2Info_ [req] = info;
						connect (req,
								SIGNAL (finished ()),
								this,
								SLOT (handlePhotosSaved ()));
					}, this);
			});
	}

	void UploadManager::handlePhotosSaved ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& item = PhotoSave2Info_.take (reply);

		const auto& data = reply->readAll ();
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot parse reply"
					<< data;
			return;
		}

		QStringList ids;
		auto photoElem = doc
				.documentElement ()
				.firstChildElement ("photo");
		while (!photoElem.isNull ())
		{
			ids << QString ("%1_%2")
					.arg (photoElem.firstChildElement ("owner_id").text ())
					.arg (photoElem.firstChildElement ("pid").text ());
			photoElem = photoElem.nextSiblingElement ("photo");
		}

		Acc_->Schedule ([this, ids] (const QString& authKey)
			{
				QUrl getUrl ("https://api.vk.com/method/photos.getById.xml");
				Util::UrlOperator { getUrl }
						("photos", ids.join (","))
						("photo_sizes", "1")
						("access_token", authKey);
				RequestQueue_->Schedule ([this, getUrl]
					{
						connect (Proxy_->GetNetworkAccessManager ()->get (QNetworkRequest (getUrl)),
								SIGNAL (finished ()),
								Acc_,
								SLOT (handlePhotosInfosFetched ()));
					}, this);
			});

		// TODO remember to add url
		emit itemUploaded (item, {});
	}
}
}
}
