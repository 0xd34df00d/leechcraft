/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "uploadmanager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QFileInfo>
#include <util/util.h>
#include <util/sll/queuemanager.h>
#include "picasaaccount.h"

namespace LC
{
namespace Blasq
{
namespace Vangog
{
	UploadManager::UploadManager (Util::QueueManager *reqQueue,
			ICoreProxy_ptr proxy, PicasaAccount *acc)
	: QObject (acc)
	, Account_ (acc)
	, Proxy_ (proxy)
	, RequestQueue_ (reqQueue)
	{
	}

	void UploadManager::Upload (const QByteArray& albumId, const QList<UploadItem>& items)
	{
		Account_->Schedule ([this, items, albumId] (const QString& authKey) -> void
			{
				QString urlStr = QString ("https://picasaweb.google.com/data/feed/api/user/%1/albumid/%2?access_token=%3")
						.arg (Account_->GetLogin ())
						.arg (albumId.isEmpty () ? "default" : QString::fromUtf8 (albumId))
						.arg (authKey);

				for (const auto& item : items)
				{
					RequestQueue_->Schedule ([this, urlStr, item] () -> void
						{
							QNetworkRequest request { urlStr };
							request.setHeader (QNetworkRequest::ContentTypeHeader, "image/jpeg");
							request.setRawHeader ("Slug",
									QFileInfo (item.FilePath_).fileName ().toUtf8 ());
							QFile *file = new QFile (item.FilePath_);
							if (!file->open (QIODevice::ReadOnly))
							{
								qWarning () << Q_FUNC_INFO
										<< "unable to open file"
										<< item.FilePath_
										<< ":"
										<< file->errorString ();
								return;
							}
							auto reply = Proxy_->GetNetworkAccessManager ()->
									post (request, file);
							Reply2Item_ [reply] = item;
							file->setParent (reply);
							connect (reply,
									SIGNAL (uploadProgress (qint64, qint64)),
									this,
									SLOT (handleUploadProgress (qint64, qint64)));
							connect (reply,
									SIGNAL (finished ()),
									this,
									SLOT (handleUploadFinished ()));
							connect (reply,
									SIGNAL (error (QNetworkReply::NetworkError)),
									this,
									SLOT (handleNetworkError (QNetworkReply::NetworkError)));
						}, this);
				}
			});
	}

	void UploadManager::handleUploadProgress (qint64 sent, qint64 total)
	{
		qDebug () << Q_FUNC_INFO << sent << total;
	}

	void UploadManager::handleUploadFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		Account_->ImageUploadResponse (reply->readAll (), Reply2Item_.take (reply));
		reply->deleteLater ();
	}

	void UploadManager::handleNetworkError (QNetworkReply::NetworkError error)
	{
		auto reply = qobject_cast<QNetworkReply *> (sender ());
		QString errorText;
		if (reply)
		{
			errorText = reply->errorString ();
			reply->deleteLater ();
		}
		emit gotError (error, errorText);
	}
}
}
}
