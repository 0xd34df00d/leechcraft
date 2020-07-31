/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATERSERVICE_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATERSERVICE_H

#include <QObject>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/structures.h>
#include <interfaces/ibookmarksservice.h>

class QNetworkReply;

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace ReadItLater
{

	class ReadItLaterApi;
	class ReadItLaterAccount;

	class ReadItLaterService : public QObject
							, public IBookmarksService
	{
		Q_OBJECT
		Q_INTERFACES (LC::Poshuku::OnlineBookmarks::IBookmarksService)
	public:
		enum OperationType
		{
			OTAuth,
			OTRegister,
			OTDownload,
			OTUpload
		};

		struct Request
		{
			OperationType Type_;
			QString Login_;
			QString Password_;
		};
	private:
		ICoreProxy_ptr CoreProxy_;
		std::shared_ptr<ReadItLaterApi> ReadItLaterApi_;
		QHash<QNetworkReply*, Request> Reply2Request_;
		QList<ReadItLaterAccount*> Accounts_;
		QHash<IAccount*, QByteArray> Account2ReplyContent_;
	public:
		explicit ReadItLaterService (ICoreProxy_ptr);

		void Prepare ();

		IBookmarksService::Features GetFeatures () const override;
		QObject* GetQObject () override;
		QString GetServiceName () const override;
		QIcon GetServiceIcon () const override;
		QWidget* GetAuthWidget () override;
		void CheckAuthData (const QVariantMap&) override;
		void RegisterAccount (const QVariantMap&) override;
		void UploadBookmarks (QObject*, const QVariantList&) override;
		void DownloadBookmarks (QObject*, const QDateTime& from = QDateTime ()) override;

		ReadItLaterAccount* GetAccountByName (const QString&);
	private:
		void SendRequest (const QString&, const QByteArray&, Request);
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

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATERSERVICE_H
