#ifndef CORE_H
#define CORE_H
#include <memory>
#include <QAbstractItemModel>
#include <QString>
#include <QMap>
#include <QDateTime>
#include <interfaces/interfaces.h>
#include <boost/shared_ptr.hpp>
#include "item.h"
#include "channel.h"
#include "feed.h"
#include "storagebackend.h"

class ChannelsModel;
class TagsCompletionModel;
class QTimer;
class QNetworkReply;
class QFile;

class Core : public QAbstractItemModel
{
    Q_OBJECT

    Core ();
    QMap<QString, QObject*> Providers_;

    enum Columns
    {
        ColumnName = 0 , ColumnDate = 1
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

    QMap<QString, Feed_ptr> Feeds_;
    Channel *ActivatedChannel_;
    QStringList ItemHeaders_;
    ChannelsModel *ChannelsModel_;
    TagsCompletionModel *TagsCompletionModel_;
    QTimer *UpdateTimer_;
    bool SaveScheduled_;
	std::auto_ptr<StorageBackend> StorageBackend_;
public:
	struct ChannelInfo
	{
		QString Link_;
		QString Description_;
		QString Author_;
	};

    static Core& Instance ();
    void Release ();
    void DoDelayedInit ();
    void SetProvider (QObject*, const QString&);
    void AddFeed (const QString&, const QStringList&);
    void RemoveFeed (const QModelIndex&);
    void Activated (const QModelIndex&);
    void FeedActivated (const QModelIndex&);
    QString GetDescription (const QModelIndex&);
	QString GetAuthor (const QModelIndex&);
	QString GetCategory (const QModelIndex&);
	QString GetLink (const QModelIndex&);
	QDateTime GetPubDate (const QModelIndex&);
    QAbstractItemModel* GetChannelsModel ();
    TagsCompletionModel* GetTagsCompletionModel ();
    void UpdateTags (const QStringList&);
    void MarkItemAsUnread (const QModelIndex&);
	bool IsItemRead (int) const;
    void MarkChannelAsRead (const QModelIndex&);
    void MarkChannelAsUnread (const QModelIndex&);
    QStringList GetTagsForIndex (int) const;
	ChannelInfo GetChannelInfo (const QModelIndex&) const;
	QPixmap GetChannelPixmap (const QModelIndex&) const;
    void SetTagsForIndex (const QString&, const QModelIndex&);
    void UpdateFeed (const QModelIndex&);
    QModelIndex GetUnreadChannelIndex ();
	void AddToItemBucket (const QModelIndex&) const;
	void AddFromOPML (const QString&,
			const QString&,
			const std::vector<bool>&);
	void ExportToOPML (const QString&) const;

    virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
    virtual QVariant data (const QModelIndex&,
			int = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual bool hasChildren (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation,
			int = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int,
			const QModelIndex& = QModelIndex()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
public slots:
    void currentChannelChanged (const QModelIndex&);
    void updateFeeds ();
private slots:
    void fetchExternalFile (const QString&, const QString&);
    void scheduleSave ();
    void handleJobFinished (int);
    void handleJobRemoved (int);
    void handleJobError (int, IDirectDownload::Error);
    void saveSettings ();
public slots:
    void updateIntervalChanged ();
    void showIconInTrayChanged ();
    void handleSslError (QNetworkReply*);
private:
    QString FindFeedForChannel (const Channel_ptr&) const;
    void UpdateUnreadItemsNumber () const;
	void FetchPixmap (const Channel_ptr&);
	void FetchFavicon (const Channel_ptr&);
	void HandleExternalData (const QString&, const QFile&);
	QString HandleFeedUpdated (const channels_container_t&,
			const PendingJob&);
signals:
    void error (const QString&);
    void showDownloadMessage (const QString&);
    void channelDataUpdated ();
    void unreadNumberChanged (int) const;
};

#endif

