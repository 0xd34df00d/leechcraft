#include <QtDebug>
#include <QApplication>
#include <QFont>
#include <QPalette>
#include <plugininterface/treeitem.h>
#include "channelsmodel.h"
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
        return static_cast<TreeItem*> (index.internalPointer ())->Data (2).toInt () ?
		   	Qt::red : QApplication::palette ().color (QPalette::Text);
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

void ChannelsModel::AddChannel (const ChannelShort& channel)
{
    beginInsertRows (QModelIndex (), rowCount (), rowCount ());

	QList<QVariant> data;
	data << channel.Title_ << channel.LastBuild_ << channel.Unread_;

	TreeItem *channelItem = new TreeItem (data, RootItem_);
	channelItem->ModifyData (0, channel.Favicon_, Qt::DecorationRole);

	RootItem_->AppendChild (channelItem);

	Channel2TreeItem_ [channel] = channelItem;
	TreeItem2Channel_ [channelItem] = channel;

    endInsertRows ();
}

void ChannelsModel::Update (const channels_container_t& channels)
{
    QList<ChannelShort> channelswh = Channel2TreeItem_.keys ();
    for (size_t i = 0; i < channels.size (); ++i)
    {
        bool found = false;
        for (int j = 0; j < channelswh.size (); ++j)
            if (channels.at (i)->ToShort () == channelswh.at (j))
			{
                found = true;
				break;
			}
        if (found)
            continue;

        QList<QVariant> data;
        Channel_ptr current = channels.at (i);
        data << current->Title_ << current->LastBuild_ << current->CountUnreadItems ();

        TreeItem *channelItem = new TreeItem (data, RootItem_);
        RootItem_->AppendChild (channelItem);

		ChannelShort cs = current->ToShort ();

        Channel2TreeItem_ [cs] = channelItem;
        TreeItem2Channel_ [channelItem] = cs;
    }
}

void ChannelsModel::UpdateChannelData (const ChannelShort& cs)
{
    Channel2TreeItemDictionary_t::iterator position = Channel2TreeItem_.end ();
    for (Channel2TreeItemDictionary_t::iterator i =
			Channel2TreeItem_.begin (), end = Channel2TreeItem_.end ();
			i != end; ++i)
        if (i.key () == cs)
        {
            position = i;
            break;
        }

    if (position == Channel2TreeItem_.end ())
        return;

    TreeItem *item = position.value ();

	TreeItem2Channel_ [item] = cs;

	item->ModifyData (0, cs.Favicon_, Qt::DecorationRole);
    item->ModifyData (1, cs.LastBuild_);
    item->ModifyData (2, cs.Unread_);
    int pos = RootItem_->ChildPosition (item);
    emit dataChanged (index (pos, 0), index (pos, 2));
    emit channelDataUpdated ();
}

ChannelShort& ChannelsModel::GetChannelForIndex (const QModelIndex& index)
{
	return TreeItem2Channel_ [static_cast<TreeItem*> (index.internalPointer ())];
}

void ChannelsModel::RemoveChannel (const ChannelShort& channel)
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

QModelIndex ChannelsModel::GetUnreadChannelIndex ()
{
    for (int i = 0; i < RootItem_->ChildCount (); ++i)
    {
        TreeItem *item = RootItem_->Child (i);
        ChannelShort channel = TreeItem2Channel_ [item];
        if (channel.Unread_)
            return index (i, 0);
    }
    return QModelIndex ();
}

