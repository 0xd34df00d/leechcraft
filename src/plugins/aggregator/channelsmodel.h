#ifndef CHANNELSMODEL_H
#define CHANNELSMODEL_H
#include <QAbstractItemModel>
#include <boost/shared_ptr.hpp>
#include "channel.h"

class TreeItem;
struct Channel;

class ChannelsModel : public QAbstractItemModel
{
    Q_OBJECT

    TreeItem *RootItem_;
    typedef QMap<ChannelShort, TreeItem*> Channel2TreeItemDictionary_t;
    typedef QMap<TreeItem*, ChannelShort> TreeItem2ChannelDictionary_t;
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

    void AddChannel (const ChannelShort&);
    void Update (const channels_container_t&);
    void UpdateChannelData (const ChannelShort&);
    ChannelShort& GetChannelForIndex (const QModelIndex&);
    void RemoveChannel (const ChannelShort&);
    QModelIndex GetUnreadChannelIndex () const;
	int GetUnreadItemsNumber () const;
signals:
    void channelDataUpdated ();
};

#endif

