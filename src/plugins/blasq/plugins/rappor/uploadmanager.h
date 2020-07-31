/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <interfaces/core/icoreproxy.h>

class QUrl;

namespace LC
{
namespace Util
{
	class QueueManager;
}

namespace Blasq
{
struct UploadItem;

namespace Rappor
{
	class VkAccount;

	class UploadManager : public QObject
	{
		Q_OBJECT

		VkAccount * const Acc_;
		const ICoreProxy_ptr Proxy_;
		Util::QueueManager * const RequestQueue_;

		QHash<QNetworkReply*, QList<UploadItem>> PhotosUploadServer2Infos_;
		QHash<QNetworkReply*, QList<UploadItem>> PhotoUpload2QueueTail_;
		QHash<QNetworkReply*, QString> PhotoUpload2Server_;
		QHash<QNetworkReply*, UploadItem> PhotoUpload2Info_;

		QHash<QNetworkReply*, UploadItem> PhotoSave2Info_;
	public:
		UploadManager (Util::QueueManager *reqQueue, ICoreProxy_ptr, VkAccount *acc);

		void Upload (const QString&, const QList<UploadItem>&);
	private:
		void StartUpload (const QString& server, QList<UploadItem> tail);
	private slots:
		void handlePhotosUploadServer ();
		void handlePhotosUploadProgress (qint64, qint64);
		void handlePhotosUploaded ();
		void handlePhotosSaved ();
	signals:
		void itemUploaded (const UploadItem&, const QUrl&);
	};
}
}
}
