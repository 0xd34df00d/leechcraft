#ifndef CORE_H
#define CORE_H
#include <memory>
#include <QAbstractItemModel>
#include <QString>
#include <QMap>
#include <QPair>
#include <QDateTime>
#include <interfaces/idownload.h>
#include <boost/shared_ptr.hpp>
#include "item.h"
#include "channel.h"
#include "feed.h"
#include "storagebackend.h"

class ChannelsModel;
class QTimer;
class QNetworkReply;
class QFile;
class ItemModel;
class QWebView;
class JobHolderRepresentation;
class IWebBrowser;
class ChannelsFilterModel;
class QSortFilterProxyModel;
class ItemsListModel;
class QToolBar;

namespace LeechCraft
{
	namespace Util
	{
		class TagsCompletionModel;
		class MergeModel;
	};
};

class Core : public QObject
{
    Q_OBJECT

	QMap<QString, QObject*> Providers_;

    enum Columns
    {
        ColumnName = 0
        , ColumnDate = 1
    };

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

    ChannelsModel *ChannelsModel_;
	LeechCraft::Util::TagsCompletionModel *TagsCompletionModel_;
    QTimer *UpdateTimer_, *CustomUpdateTimer_;
    bool SaveScheduled_;
	boost::shared_ptr<StorageBackend> StorageBackend_;
    ItemModel *ItemModel_;
	JobHolderRepresentation *JobHolderRepresentation_;
	QMap<QString, QDateTime> Updates_;
	ChannelsFilterModel *ChannelsFilterModel_;
	ItemsListModel *CurrentItemsModel_;
	QList<boost::shared_ptr<ItemsListModel> > SupplementaryModels_;
	LeechCraft::Util::MergeModel *ItemLists_;
	bool MergeMode_;

    Core ();
public:
	struct ChannelInfo
	{
		QString Link_;
		QString Description_;
		QString Author_;
	};

    static Core& Instance ();
    void Release ();
	bool CouldHandle (const LeechCraft::DownloadEntity&);
	void SetWidgets (QToolBar*, QWidget*);
    void DoDelayedInit ();
    void SetProvider (QObject*, const QString&);
    void AddFeed (const QString&, const QStringList&);
    void RemoveFeed (const QModelIndex&, bool);
	void Selected (const QModelIndex&);
    Item_ptr GetItem (const QModelIndex&) const;
    QSortFilterProxyModel* GetChannelsModel () const;
	QAbstractItemModel* GetItemsModel () const;
	LeechCraft::Util::TagsCompletionModel* GetTagsCompletionModel () const;
	IWebBrowser* GetWebBrowser () const;
    void UpdateTags (const QStringList&);
    void MarkItemAsUnread (const QModelIndex&);
	bool IsItemRead (int) const;
	bool IsItemCurrent (int) const;
    void MarkChannelAsRead (const QModelIndex&);
    void MarkChannelAsUnread (const QModelIndex&);
    QStringList GetTagsForIndex (int) const;
	ChannelInfo GetChannelInfo (const QModelIndex&) const;
	QPixmap GetChannelPixmap (const QModelIndex&) const;
	void SetTagsForIndex (const QString&, const QModelIndex&);
	QStringList GetCategories (const QModelIndex&) const;
	QStringList GetItemCategories (int) const;
	Feed::FeedSettings GetFeedSettings (const QModelIndex&) const;
	void SetFeedSettings (const Feed::FeedSettings&, const QModelIndex&);
    void UpdateFeed (const QModelIndex&, bool);
    QModelIndex GetUnreadChannelIndex () const;
	int GetUnreadChannelsNumber () const;
	void AddToItemBucket (const QModelIndex&) const;
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
    ItemModel* GetItemModel () const;
	StorageBackend* GetStorageBackend () const;
	void SubscribeToComments (const QModelIndex&);
	QWebView* CreateWindow ();
	void GetChannels (channels_shorts_t&) const;
	void AddFeeds (const feeds_container_t&, const QString&);
	void SetMerge (bool);
    void CurrentChannelChanged (const QModelIndex&, bool);
public slots:
	void openLink (const QString&);
    void updateFeeds ();
    void updateIntervalChanged ();
    void showIconInTrayChanged ();
    void handleSslError (QNetworkReply*);
	void tagsUpdated ();
private slots:
    void fetchExternalFile (const QString&, const QString&);
    void scheduleSave ();
    void handleJobFinished (int);
    void handleJobRemoved (int);
    void handleJobError (int, IDownload::Error);
    void saveSettings ();
	void handleChannelDataUpdated (Channel_ptr);
	void handleItemDataUpdated (Item_ptr, Channel_ptr);
	void handleCustomUpdates ();
private:
    void UpdateUnreadItemsNumber () const;
	void FetchPixmap (const Channel_ptr&);
	void FetchFavicon (const Channel_ptr&);
	void HandleExternalData (const QString&, const QFile&);
	QString HandleFeedUpdated (const channels_container_t&,
			const channels_container_t&,
			const channels_container_t&,
			const PendingJob&);
	void MarkChannel (const QModelIndex&, bool);
	void UpdateFeed (const QString&);
	void HandleProvider (QObject*);
signals:
    void error (const QString&) const;
    void showDownloadMessage (const QString&);
    void channelDataUpdated ();
	void currentChannelChanged (const QModelIndex&);
    void unreadNumberChanged (int) const;
	void delegateEntity (const LeechCraft::DownloadEntity&, int*, QObject**);
};

#endif

