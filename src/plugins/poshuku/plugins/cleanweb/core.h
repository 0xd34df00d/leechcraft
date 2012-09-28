/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#pragma once

#include <QAbstractItemModel>
#include <QHash>
#include <QStringList>
#include <QNetworkReply>
#include <QDateTime>
#include <QWebPage>
#include <interfaces/iinfo.h>
#include <interfaces/idownload.h>
#include <interfaces/poshukutypes.h>
#include <interfaces/core/ihookproxy.h>
#include "filter.h"

class QNetworkRequest;
class QWebPage;
class QGraphicsWebView;
class QWebHitTestResult;

namespace LeechCraft
{
namespace Poshuku
{
namespace CleanWeb
{
	class FlashOnClickPlugin;
	class FlashOnClickWhitelist;
	class UserFiltersModel;

	class Core : public QAbstractItemModel
	{
		Q_OBJECT

		FlashOnClickPlugin *FlashOnClickPlugin_;
		FlashOnClickWhitelist *FlashOnClickWhitelist_;
		UserFiltersModel *UserFilters_;

		QList<Filter> Filters_;
		QObjectList Downloaders_;
		QStringList HeaderLabels_;

		struct PendingJob
		{
			QString FullName_;
			QString FileName_;
			QString Subscr_;
			QUrl URL_;
		};
		QMap<int, PendingJob> PendingJobs_;

		QHash<QWebFrame*, QStringList> MoreDelayedURLs_;

		ICoreProxy_ptr Proxy_;

		Core ();
	public:
		static Core& Instance ();
		void Release ();

		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;

		int columnCount (const QModelIndex& = QModelIndex ()) const;
		QVariant data (const QModelIndex&, int) const;
		QVariant headerData (int, Qt::Orientation, int) const;
		QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
		QModelIndex parent (const QModelIndex&) const;
		int rowCount (const QModelIndex& = QModelIndex ()) const;

		bool CouldHandle (const Entity&) const;
		void Handle (Entity);
		QAbstractItemModel* GetModel ();
		void Remove (const QModelIndex&);

		void HandleInitialLayout (QWebPage*, QWebFrame*);
		QNetworkReply* Hook (LeechCraft::IHookProxy_ptr,
				QNetworkAccessManager*,
				QNetworkAccessManager::Operation*,
				QIODevice**);
		void HandleExtension (LeechCraft::IHookProxy_ptr,
				QWebPage*,
				QWebPage::Extension,
				const QWebPage::ExtensionOption*,
				QWebPage::ExtensionReturn*);
		void HandleContextMenu (const QWebHitTestResult&,
				QGraphicsWebView*, QMenu*,
				WebViewCtxMenuStage);

		bool ShouldReject (const QNetworkRequest&, QString*) const;

		UserFiltersModel* GetUserFiltersModel () const;
		FlashOnClickPlugin* GetFlashOnClick ();
		FlashOnClickWhitelist* GetFlashOnClickWhitelist ();

		bool Exists (const QUrl& url) const;
		bool Exists (const QString& name) const;

		/** Parses the abp:-schemed url, gets subscription
		 * name and real url from there and adds it via Load().
		 *
		 * Returns true if the url is added successfully or
		 * false otherwise (if url is malformed or such
		 * subscription already exists, for example).
		 *
		 * @param[in] url The abp:-schemed URL.
		 *
		 * @return Whether addition was successful.
		 */
		bool Add (const QUrl& url);

		/** Loads the subscription from the url with the name
		 * subscrName. Returns true if the load delegation was
		 * successful, otherwise returns false.
		 *
		 * url is expected to be a "real" URL of the filters
		 * file — with, say, http:// scheme.
		 *
		 * Returns true if the url is added successfully or
		 * false otherwise (if url is malformed or such
		 * subscription already exists, for example).
		 *
		 * @param[in] url Real URL of the file with the filters.
		 * @param[in] subscrName The name if this subscription.
		 *
		 * @return Whether addition was successful.
		 */
		bool Load (const QUrl& url, const QString& subscrName);
	private:
		bool Matches (const FilterItem&, const QString&, const QByteArray&, const QString&) const;
		void HandleProvider (QObject*);
		void Parse (const QString&);

		/** Removes the subscription at
		 * ~/.leechcraft/cleanweb/filename.
		 */
		void Remove (const QString& filename);
		void WriteSettings ();
		void ReadSettings ();
		bool AssignSD (const SubscriptionData&);
	private slots:
		void update ();
		void handleJobFinished (int);
		void handleJobError (int, IDownload::Error);
		void handleFrameLayout (QPointer<QWebFrame>);
		void delayedRemoveElements (QPointer<QWebFrame>, const QString&);
		void moreDelayedRemoveElements ();
		void handleFrameDestroyed ();
	signals:
		void delegateEntity (const LeechCraft::Entity&,
				int*, QObject**);
		void gotEntity (const LeechCraft::Entity&);
	};
}
}
}
