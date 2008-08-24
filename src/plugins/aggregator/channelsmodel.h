#ifndef CHANNELSMODEL_H
#define CHANNELSMODEL_H
#include <QAbstractItemModel>
#include <boost/shared_ptr.hpp>
#include "feed.h"

class TreeItem;
class Channel;

class ChannelsModel : public QAbstractItemModel
{
    Q_OBJECT

    TreeItem *RootItem_;
    typedef QMap<boost::shared_ptr<Channel>, TreeItem*> Channel2TreeItemDictionary_t;
    typedef QMap<TreeItem*, boost::shared_ptr<Channel> > TreeItem2ChannelDictionary_t;
    Channel2TreeItemDictionary_t Channel2TreeItem_;
    TreeItem2ChannelDictionary_t TreeItem2Channel_;
public:
    ChannelsModel (QObject *parent = 0);
    virtual ~ChannelsModel ();

    virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
    virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

    void AddFeed (const Feed&);
    void Update (const Feed::channels_container_t&);
    void UpdateChannelData (const Channel*);
    void UpdateChannelData (const boost::shared_ptr<Channel>&);
    boost::shared_ptr<Channel>& GetChannelForIndex (const QModelIndex&);
    void RemoveChannel (const boost::shared_ptr<Channel>&);
    void MarkChannelAsRead (const QModelIndex&);
    void MarkChannelAsUnread (const QModelIndex&);
    QModelIndex GetUnreadChannelIndex ();
signals:
    void channelDataUpdated ();
};

#endif

