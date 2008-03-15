#include "treeitem.h"
#include "channelsmodel.h"
#include "channel.h"
#include "feed.h"

ChannelsModel::ChannelsModel (QObject *parent)
: QAbstractItemModel (parent)
{
    QVariantList roots;
    roots << tr ("Feeds");
    RootItem_ = new TreeItem (roots);
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
    if (role != Qt::DisplayRole)
        return QVariant ();

    return static_cast<TreeItem*> (index.internalPointer ())->Data (index.column ());
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
    QList<Channel*> channels = feed.Channels_;
    beginInsertRows (QModelIndex (), rowCount (), rowCount () + channels.size () - 1);
    for (int i = 0; i < channels.size (); ++i)
    {
        QList<QVariant> data;
        Channel *current = channels.at (i);
        data << current->Title_ << (current->LastBuild_.isValid () ? current->LastBuild_ : feed.LastUpdate_);
        TreeItem *channelItem = new TreeItem (data, RootItem_);
        RootItem_->AppendChild (channelItem);
        Channel2TreeItem_ [current] = channelItem;
        TreeItem2Channel_ [channelItem] = current;
    }
    endInsertRows ();
}

Channel* ChannelsModel::GetChannelForIndex (const QModelIndex& index) const
{
    if (index.isValid ())
        return TreeItem2Channel_ [static_cast<TreeItem*> (index.internalPointer ())];
    else
        return 0;
}

