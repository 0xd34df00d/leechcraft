#ifndef CORE_H
#define CORE_H
#include <QAbstractItemModel>
#include <QString>
#include <QMap>
#include <QDateTime>
#include <interfaces/interfaces.h>
#include "item.h"
#include "channel.h"
#include "feed.h"

class TreeItem;

class Core : public QAbstractItemModel
{
    Q_OBJECT

    Core ();
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
        } Role_;
        QString URL_;
        QString Filename_;
    };
    QMap<int, PendingJob> PendingJobs_;

    QMap<QString, Feed> Feeds_;
    QMap<Channel*, TreeItem*> Channel2TreeItem_;
    QMap<Item*, TreeItem*> Item2TreeItem_;
    QMap<Item*, bool> ItemUnread_;
    QMap<TreeItem*, Item*> TreeItem2Item_;
    TreeItem *RootItem_;
public:
    static Core& Instance ();
    void Release ();
    void SetProvider (QObject*, const QString&);
    void AddFeed (const QString&);
    void Activated (const QModelIndex&);
    QString GetDescription (const QModelIndex&);

    virtual int columnCount (const QModelIndex& parent = QModelIndex ()) const;
    virtual QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation, int role = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& parent = QModelIndex ()) const;
private slots:
    void handleJobFinished (int);
    void handleJobRemoved (int);
    void handleJobError (int, IDirectDownload::Error);
signals:
    void error (const QString&);
};

#endif

