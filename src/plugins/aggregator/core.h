#ifndef CORE_H
#define CORE_H
#include <QAbstractItemModel>
#include <QString>
#include <QMap>
#include <QDateTime>
#include <interfaces/interfaces.h>
#include <boost/shared_ptr.hpp>
#include "item.h"
#include "channel.h"
#include "feed.h"

class ChannelsModel;
class QTimer;

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
        } Role_;
        QString URL_;
        QString Filename_;
        QStringList Tags_;
    };
    QMap<int, PendingJob> PendingJobs_;

    QMap<QString, Feed> Feeds_;
    Channel *ActivatedChannel_;
    QStringList ItemHeaders_;
    ChannelsModel *ChannelsModel_;
    QTimer *UpdateTimer_;
    bool SaveScheduled_;
public:
    static Core& Instance ();
    void Release ();
    void DoDelayedInit ();
    void SetProvider (QObject*, const QString&);
    void AddFeed (const QString&, const QStringList&);
    void RemoveFeed (const QModelIndex&);
    void Activated (const QModelIndex&);
    void FeedActivated (const QModelIndex&);
    QString GetDescription (const QModelIndex&);
    QAbstractItemModel* GetChannelsModel ();
    void MarkItemAsUnread (const QModelIndex&);
    void MarkChannelAsRead (const QModelIndex&);
    void MarkChannelAsUnread (const QModelIndex&);
    QStringList GetTagsForIndex (int) const;
    QString GetChannelLink (const QModelIndex&) const;
    QString GetChannelDescription (const QModelIndex&) const;
    QString GetChannelAuthor (const QModelIndex&) const;
    QString GetChannelLanguage (const QModelIndex&) const;
    void SetTagsForIndex (const QString&, const QModelIndex&);

    virtual int columnCount (const QModelIndex& parent = QModelIndex ()) const;
    virtual QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual bool hasChildren (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation, int role = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& parent = QModelIndex ()) const;
public slots:
    void currentChannelChanged (const QModelIndex&);
    void updateFeeds ();
private slots:
    void scheduleSave ();
    void handleJobFinished (int);
    void handleJobRemoved (int);
    void handleJobError (int, IDirectDownload::Error);
    void saveSettings ();
public slots:
    void updateIntervalChanged ();
signals:
    void error (const QString&);
    void showDownloadMessage (const QString&);
};

#endif

