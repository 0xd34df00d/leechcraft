/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_DELICIOUS_DELICIOUSSERVICE_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_DELICIOUS_DELICIOUSSERVICE_H

#include <QObject>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/structures.h>
#include <interfaces/ibookmarksservice.h>

namespace LeechCraft
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
		Q_INTERFACES (LeechCraft::Poshuku::OnlineBookmarks::IBookmarksService)

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
		DeliciousService (ICoreProxy_ptr);
		void Prepare ();
		IBookmarksService::Features GetFeatures () const;
		QObject* GetObject ();
		QString GetServiceName () const;
		QIcon GetServiceIcon () const;
		QWidget* GetAuthWidget ();
		void CheckAuthData (const QVariantMap&);
		void RegisterAccount (const QVariantMap&);
		void UploadBookmarks (QObject*, const QVariantList&);
		void DownloadBookmarks (QObject*, const QDateTime&);
		DeliciousAccount* GetAccountByName (const QString&);
	private:
		void SendRequest (const QString&, const QByteArray&, const Request&);
		void RestoreAccounts ();
	private slots:
		void getReplyFinished ();
		void readyReadReply ();
		void saveAccounts () const;
		void removeAccount (QObject*);
	signals:
		void accountAdded (QObjectList);
		void gotEntity (const LeechCraft::Entity&);
		void gotBookmarks (QObject*, const QVariantList&);
		void bookmarksUploaded ();
	};
}
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_DELICIOUS_DELICIOUSSERVICE_H
