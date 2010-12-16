/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGINS_AGGREGATOR_CORE_H
#define PLUGINS_AGGREGATOR_CORE_H
#include <memory>
#include <boost/shared_ptr.hpp>
#include <QAbstractItemModel>
#include <QString>
#include <QMap>
#include <QPair>
#include <QDateTime>
#include <interfaces/iinfo.h>
#include <interfaces/idownload.h>
#include <plugininterface/idpool.h>
#include "item.h"
#include "channel.h"
#include "feed.h"
#include "storagebackend.h"
#include "actionsstructs.h"

class QTimer;
class QNetworkReply;
class QFile;
class QWebView;
class QSortFilterProxyModel;
class QToolBar;
class IWebBrowser;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class ChannelsModel;
			class JobHolderRepresentation;
			class ChannelsFilterModel;
			class ItemsWidget;

			class Core : public QObject
			{
				Q_OBJECT

				QMap<QString, QObject*> Providers_;

				enum Columns
				{
					ColumnName = 0
					, ColumnDate = 1
				};

				struct PendingOPML
				{
					QString Filename_;
				};
				QMap<int, PendingOPML> PendingOPMLs_;

				struct PendingJob
				{
					enum Role
					{
						RFeedAdded
						, RFeedUpdated
						, RFeedExternalData
					} Role_;
					QString URL_;
					QString Filename_;
					QStringList Tags_;
					boost::shared_ptr<Feed::FeedSettings> FeedSettings_;
				};
				struct ExternalData
				{
					enum Type
					{
						TImage
						, TIcon
					} Type_;
					Channel_ptr RelatedChannel_;
					Feed_ptr RelatedFeed_;
				};
				QMap<int, PendingJob> PendingJobs_;
				QMap<QString, ExternalData> PendingJob2ExternalData_;
				QList<QObject*> Downloaders_;
				QMap<int, QObject*> ID2Downloader_;

				bool SaveScheduled_;
				ChannelsModel *ChannelsModel_;
				QTimer *UpdateTimer_, *CustomUpdateTimer_;
				boost::shared_ptr<StorageBackend> StorageBackend_;
				JobHolderRepresentation *JobHolderRepresentation_;
				QMap<IDType_t, QDateTime> Updates_;
				ChannelsFilterModel *ChannelsFilterModel_;
				ICoreProxy_ptr Proxy_;
				bool Initialized_;
				AppWideActions AppWideActions_;
				ItemsWidget *ReprWidget_;

				Core ();
			private:
				QHash<PoolType, Util::IDPool<IDType_t> > Pools_;
			public:
				struct ChannelInfo
				{
					IDType_t FeedID_;
					IDType_t ChannelID_;
					QString URL_;
					QString Link_;
					QString Description_;
					QString Author_;
					int NumItems_;
				};

				static Core& Instance ();
				void Release ();

				void SetProxy (ICoreProxy_ptr);
				ICoreProxy_ptr GetProxy () const;

				Util::IDPool<IDType_t>& GetPool (PoolType);

				bool CouldHandle (const LeechCraft::Entity&);
				void Handle (LeechCraft::Entity);
				void StartAddingOPML (const QString&);
				void SetAppWideActions (const AppWideActions&);
				const AppWideActions& GetAppWideActions () const;
				bool DoDelayedInit ();
				void AddFeed (const QString&, const QString&);
				void AddFeed (const QString&, const QStringList&,
						FeedSettings_ptr = FeedSettings_ptr ());
				void RemoveFeed (const QModelIndex&);
				ItemsWidget* GetReprWidget () const;

				/** Returns the channels model as it is.
				 *
				 * @sa GetRawChannelsModel
				 */
				ChannelsModel* GetRawChannelsModel () const;

				/** Returns the filter model with the
				 * GetRawChannelsModel as a source.
				 *
				 * @sa GetRawChannelsModel.
				 */
				QSortFilterProxyModel* GetChannelsModel () const;
				IWebBrowser* GetWebBrowser () const;
				void MarkChannelAsRead (const QModelIndex&);
				void MarkChannelAsUnread (const QModelIndex&);

				/** Returns user-meaningful tags for the given index.
				 */
				QStringList GetTagsForIndex (int) const;
				ChannelInfo GetChannelInfo (const QModelIndex&) const;
				QPixmap GetChannelPixmap (const QModelIndex&) const;

				/** Sets the tags for index from the given user-edited string.
				 */
				void SetTagsForIndex (const QString&, const QModelIndex&);
				void UpdateFavicon (const QModelIndex&);
				QStringList GetCategories (const QModelIndex&) const;
				Feed::FeedSettings GetFeedSettings (const QModelIndex&) const;
				void SetFeedSettings (const Feed::FeedSettings&, const QModelIndex&);
				void UpdateFeed (const QModelIndex&, bool);
				QModelIndex GetUnreadChannelIndex () const;
				int GetUnreadChannelsNumber () const;
				void AddFromOPML (const QString&,
						const QString&,
						const std::vector<bool>&);
				void ExportToOPML (const QString&,
						const QString&,
						const QString&,
						const QString&,
						const std::vector<bool>&) const;
				void ExportToBinary (const QString&,
						const QString&,
						const QString&,
						const QString&,
						const std::vector<bool>&) const;
				JobHolderRepresentation* GetJobHolderRepresentation () const;
				StorageBackend* GetStorageBackend () const;
				QWebView* CreateWindow ();
				void GetChannels (channels_shorts_t&) const;
				void AddFeeds (const feeds_container_t&, const QString&);
				void SetContextMenu (QMenu*);
			public slots:
				void openLink (const QString&);
				void updateFeeds ();
				void updateIntervalChanged ();
				void showIconInTrayChanged ();
				void handleSslError (QNetworkReply*);
			private slots:
				void fetchExternalFile (const QString&, const QString&);
				void scheduleSave ();
				void handleJobFinished (int);
				void handleJobRemoved (int);
				void handleJobError (int, IDownload::Error);
				void saveSettings ();
				void handleChannelDataUpdated (Channel_ptr);
				void handleCustomUpdates ();
			private:
				void UpdateUnreadItemsNumber () const;
				void FetchPixmap (const Channel_ptr&);
				void FetchFavicon (const Channel_ptr&);
				void HandleExternalData (const QString&, const QFile&);
				void HandleFeedAdded (const channels_container_t&,
						const PendingJob&);
				void HandleFeedUpdated (const channels_container_t&,
						const PendingJob&);
				void MarkChannel (const QModelIndex&, bool);
				void UpdateFeed (const IDType_t&);
				void HandleProvider (QObject*, int);
				void ErrorNotification (const QString&, const QString&, bool = true) const;
			signals:
				void channelDataUpdated ();
				void unreadNumberChanged (int) const;
				void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
				void gotEntity (const LeechCraft::Entity&);
			};
		};
	};
};

#endif

