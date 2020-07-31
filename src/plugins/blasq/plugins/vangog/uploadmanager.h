/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPointer>
#include <QStandardItem>
#include <QNetworkReply>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/blasq/isupportuploads.h>

namespace LC
{
namespace Util
{
	class QueueManager;
}

namespace Blasq
{
namespace Vangog
{
	class PicasaAccount;

	class UploadManager : public QObject
	{
		Q_OBJECT

		PicasaAccount * const Account_;
		const ICoreProxy_ptr Proxy_;
		Util::QueueManager * const RequestQueue_;

		QHash<QNetworkReply*, UploadItem> Reply2Item_;
	public:
		UploadManager (Util::QueueManager *reqQueue, ICoreProxy_ptr proxy,
				PicasaAccount *acc);
		void Upload (const QByteArray& albumId, const QList<UploadItem>& items);

	private slots:
		void handleUploadProgress (qint64 sent, qint64 total);
		void handleUploadFinished ();
		void handleNetworkError (QNetworkReply::NetworkError err);

	signals:
		void gotError (int errorCode, const QString& errorString);
	};
}
}
}
