/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/structures.h>
#include <interfaces/ibookmarksservice.h>

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace Delicious
{
	class DeliciousApi;
	class DeliciousAccount;

	class DeliciousService : public QObject
							, public IBookmarksService
	{
		Q_OBJECT
		Q_INTERFACES (LC::Poshuku::OnlineBookmarks::IBookmarksService)

	public:
		enum OperationType
		{
			OTAuth,
			OTDownload,
			OTUpload
		};

		struct Request
		{
			OperationType Type_;
			QString Login_;
			QString Password_;
			int Count_;
			int Current_;
		};
	private:
		ICoreProxy_ptr CoreProxy_;
		std::shared_ptr<DeliciousApi> DeliciousApi_;
		QList<DeliciousAccount*> Accounts_;
		QHash<QNetworkReply*, Request> Reply2Request_;
		QHash<IAccount*, QByteArray> Account2ReplyContent_;
	public:
		explicit DeliciousService (ICoreProxy_ptr);

		void Prepare ();

		IBookmarksService::Features GetFeatures () const override;
		QObject* GetQObject () override;
		QString GetServiceName () const override;
		QIcon GetServiceIcon () const override;
		QWidget* GetAuthWidget () override;
		void CheckAuthData (const QVariantMap&) override;
		void RegisterAccount (const QVariantMap&) override;
		void UploadBookmarks (QObject*, const QVariantList&) override;
		void DownloadBookmarks (QObject*, const QDateTime&) override;

		DeliciousAccount* GetAccountByName (const QString&);
	private:
		void SendRequest (const QString&, const QByteArray&, const Request&);
		void RestoreAccounts ();
	private slots:
		void getReplyFinished ();
		void readyReadReply ();

		void saveAccounts () const override;
		void removeAccount (QObject*) override;
	signals:
		void accountAdded (QObjectList) override;
		void gotBookmarks (QObject*, const QVariantList&) override;
		void bookmarksUploaded () override;
	};
}
}
}
}
