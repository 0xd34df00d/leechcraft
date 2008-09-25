#include <QtDebug>
#include <QApplication>
#include <QFont>
#include <plugininterface/treeitem.h>
#include "channelsmodel.h"
#include "channel.h"
#include "feed.h"
#include "item.h"

ChannelsModel::ChannelsModel (QObject *parent)
: QAbstractItemModel (parent)
{
    QVariantList roots;
    roots << tr ("Feed") << tr ("Last build") << tr ("Unread items");
    RootItem_ = new TreeItem (roots);
}

ChannelsModel::~ChannelsModel ()
{
    delete RootItem_;
}

int ChannelsModel::columnCount (const QModelIndex& parent) const
{
    if (parent.isValid ())
        return static_cast<TreeItem*> (parent.internalPointer ())->ColumnCount ();
    else
        return RootItem_->ColumnCount();
}

QVariant ChannelsModel::data (const QModelIndex& index, int role) const
{
    if (!index.isValid ())
        return QVariant ();

    if (role == Qt::DisplayRole)
        return static_cast<TreeItem*> (index.internalPointer ())->Data (index.column ());
	else if (role == Qt::DecorationRole)
		return static_cast<TreeItem*> (index.internalPointer ())->Data (index.column (), Qt::DecorationRole);
    else if (role == Qt::ForegroundRole)
        return static_cast<TreeItem*> (index.internalPointer ())->Data (2).toInt () ? Qt::red : Qt::black;
    else if (role == Qt::FontRole)
    {
        if (static_cast<TreeItem*> (index.internalPointer ())->Data (2).toInt ())
        {
            QFont defaultFont = QApplication::font ();
            defaultFont.setBold (true);
            return defaultFont;
        }
        else
            return QVariant ();
    }
    else
        return QVariant ();
}

Qt::ItemFlags ChannelsModel::flags (const QModelIndex& index) const
{
    if (!index.isValid ())
        return 0;
    else
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ChannelsModel::headerData (int column, Qt::Orientation orient, int role) const
{
    if (orient == Qt::Horizontal && role == Qt::DisplayRole)
        return RootItem_->Data (column);
    else
        return QVariant ();
}

QModelIndex ChannelsModel::index (int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex (row, column, parent))
        return QModelIndex ();

    TreeItem *parentItem;
    if (!parent.isValid ())
        parentItem = RootItem_;
    else
        parentItem = static_cast<TreeItem*> (parent.internalPointer ());

    TreeItem *childItem = parentItem->Child (row);
    if (childItem)
        return createIndex (row, column, childItem);
    else
        return QModelIndex ();
}

QModelIndex ChannelsModel::parent (const QModelIndex& index) const
{
    if (!index.isValid ())
        return QModelIndex ();

    TreeItem *childItem = static_cast<TreeItem*> (index.internalPointer ());
    TreeItem *parentItem = childItem->Parent ();

    if (parentItem == RootItem_)
        return QModelIndex ();
    else
        return createIndex (parentItem->Row (), 0, parentItem);
}

int ChannelsModel::rowCount (const QModelIndex& parent) const
{
    if (parent.column () > 0)
        return 0;

    TreeItem *parentItem;
    if (!parent.isValid ())
        parentItem = RootItem_;
    else
        parentItem = static_cast<TreeItem*> (parent.internalPointer ());

    return parentItem->ChildCount ();
}

void ChannelsModel::AddFeed (const Feed& feed)
{
    const Feed::channels_container_t& channels = feed.Channels_;
    if (!channels.size ())
        return;

    beginInsertRows (QModelIndex (), rowCount (), rowCount () + channels.size () - 1);
    for (size_t i = 0; i < channels.size (); ++i)
    {
        QList<QVariant> data;
        const boost::shared_ptr<Channel>& current = channels.at (i);
        data << current->Title_ << current->LastBuild_ << current->CountUnreadItems ();
        TreeItem *channelItem = new TreeItem (data, RootItem_);
        RootItem_->AppendChild (channelItem);
        Channel2TreeItem_ [current] = channelItem;
        TreeItem2Channel_ [channelItem] = current;
    }
    endInsertRows ();
}

void ChannelsModel::Update (const Feed::channels_container_t& channels)
{
    QList<boost::shared_ptr<Channel> > channelswh = Channel2TreeItem_.keys ();
    for (size_t i = 0; i < channels.size (); ++i)
    {
        bool found = false;
        for (int j = 0; j < channelswh.size (); ++j)
            if (*channels.at (i) == *channelswh.at (j))
                found = true;
        if (found)
            continue;
        QList<QVariant> data;
        boost::shared_ptr<Channel> current = channels.at (i);
        data << current->Title_ << current->LastBuild_ << current->CountUnreadItems ();
        TreeItem *channelItem = new TreeItem (data, RootItem_);
        RootItem_->AppendChild (channelItem);
        Channel2TreeItem_ [current] = channelItem;
        TreeItem2Channel_ [channelItem] = current;
    }
}

void ChannelsModel::UpdateChannelData (const Channel* channel)
{
    Channel2TreeItemDictionary_t::const_iterator position = Channel2TreeItem_.end ();
    for (Channel2TreeItemDictionary_t::const_iterator i = Channel2TreeItem_.begin (); i != Channel2TreeItem_.end (); ++i)
        if (i.key ().get () == channel)
        {
            position = i;
            break;
        }

    if (position == Channel2TreeItem_.end ())
        return;
    
    TreeItem *item = position.value ();
	item->ModifyData (0, channel->Favicon_, Qt::DecorationRole);
    item->ModifyData (1, channel->LastBuild_);
    item->ModifyData (2, channel->CountUnreadItems ());
    int pos = RootItem_->ChildPosition (item);
    emit dataChanged (index (pos, 1), index (pos, 2));
    emit channelDataUpdated ();
}

void ChannelsModel::UpdateChannelData (const boost::shared_ptr<Channel>& channel)
{
    UpdateChannelData (channel.get ());
}

boost::shared_ptr<Channel>& ChannelsModel::GetChannelForIndex (const QModelIndex& index)
{
	return TreeItem2Channel_ [static_cast<TreeItem*> (index.internalPointer ())];
}

void ChannelsModel::RemoveChannel (const boost::shared_ptr<Channel>& channel)
{
    if (!Channel2TreeItem_.contains (channel))
        return;

    TreeItem *container = Channel2TreeItem_ [channel];
    int pos = RootItem_->ChildPosition (container);

    beginRemoveRows (QModelIndex (), pos, pos);
    Channel2TreeItem_.remove (channel);
    TreeItem2Channel_.remove (container);
    RootItem_->RemoveChild (pos);
    endRemoveRows ();
}

void ChannelsModel::MarkChannelAsRead (const QModelIndex& index)
{
    TreeItem *item = static_cast<TreeItem*> (index.internalPointer ());
    boost::shared_ptr<Channel> channel = TreeItem2Channel_ [item];
    for (size_t i = 0; i < channel->Items_.size (); ++i)
        channel->Items_ [i]->Unread_ = false;

    UpdateChannelData (channel);
}

void ChannelsModel::MarkChannelAsUnread (const QModelIndex& index)
{
    TreeItem *item = static_cast<TreeItem*> (index.internalPointer ());
    boost::shared_ptr<Channel> channel = TreeItem2Channel_ [item];
    for (size_t i = 0; i < channel->Items_.size (); ++i)
        channel->Items_ [i]->Unread_ = true;

    UpdateChannelData (channel);
}

QModelIndex ChannelsModel::GetUnreadChannelIndex ()
{
    for (int i = 0; i < RootItem_->ChildCount (); ++i)
    {
        TreeItem *item = RootItem_->Child (i);
        boost::shared_ptr<Channel> channel = TreeItem2Channel_ [item];
        if (channel->CountUnreadItems ())
            return index (i, 0);
    }
    return QModelIndex ();
}

