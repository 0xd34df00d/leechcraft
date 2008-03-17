#ifndef CHANNELSMODEL_H
#define CHANNELSMODEL_H
#include <QAbstractItemModel>

class TreeItem;
class Channel;
class Feed;

class ChannelsModel : public QAbstractItemModel
{
    Q_OBJECT

    TreeItem *RootItem_;
    QMap<Channel*, TreeItem*> Channel2TreeItem_;
    QMap<TreeItem*, Channel*> TreeItem2Channel_;
public:
    ChannelsModel (QObject *parent = 0);

    virtual int columnCount (const QModelIndex& parent = QModelIndex ()) const;
    virtual QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation, int role = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& parent = QModelIndex ()) const;

    void AddFeed (const Feed&);
    void Update (const QList<Channel*>&);
    void UpdateTimestamp (Channel*);
    Channel* GetChannelForIndex (const QModelIndex&) const;
};

#endif

