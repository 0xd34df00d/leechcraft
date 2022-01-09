/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include <QHash>
#include <QStringList>
#include <QNetworkReply>
#include <QDateTime>
#include <interfaces/iinfo.h>
#include <interfaces/poshuku/poshukutypes.h>
#include "filter.h"
#include "pslfetcher.h"

class QNetworkRequest;

namespace LC
{
namespace Poshuku
{
class IWebView;
class IBrowserWidget;

namespace CleanWeb
{
	class UserFiltersModel;
	class SubscriptionsModel;

	struct HidingWorkerResult
	{
		IWebView *View_;
		QStringList Selectors_;
	};

	class Core : public QObject
	{
		Q_OBJECT

		UserFiltersModel * const UserFilters_;
		SubscriptionsModel * const SubsModel_;

		QList<QList<FilterItem_ptr>> ExceptionsCache_;
		QList<QList<FilterItem_ptr>> FilterItemsCache_;

		QHash<QObject*, QSet<QUrl>> MoreDelayedURLs_;

		QSet<IWebView*> ScheduledHidings_;

		const PslFetcher PslFetcher_;

		const ICoreProxy_ptr Proxy_;
	public:
		Core (SubscriptionsModel*, UserFiltersModel*, const ICoreProxy_ptr&);

		ICoreProxy_ptr GetProxy () const;

		bool CouldHandle (const Entity&) const;
		void Handle (Entity);

		void HandleBrowserWidget (IBrowserWidget*);

		void InstallInterceptor ();

		void HandleContextMenu (const ContextMenuInfo&,
				IWebView*, QMenu*,
				WebViewCtxMenuStage);

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
		void Parse (const QString&);

		void HideElementsChunk (HidingWorkerResult);
		void DelayedRemoveElements (IWebView*, const QUrl&);
		void HandleViewLayout (IWebView*);
	private slots:
		void update ();

		void moreDelayedRemoveElements ();

		void handleViewDestroyed (QObject*);

		void regenFilterCaches ();
	};
}
}
}
