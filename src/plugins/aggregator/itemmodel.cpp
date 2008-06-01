#include <QDesktopServices>
#include <QUrl>
#include <QtDebug>
#include "item.h"
#include "itemmodel.h"

ItemModel::ItemModel (QObject *parent)
: QAbstractItemModel (parent)
{
	ItemHeaders_ << tr ("Name");
}

ItemModel::~ItemModel ()
{
}

void ItemModel::AddItem (const boost::shared_ptr<Item>& item)
{
	boost::shared_ptr<Item> local (new Item (*item));
	local->Unread_ = false;
	beginInsertRows (QModelIndex (), rowCount (), rowCount ());
	Items_.push_front (local);
	endInsertRows ();
}

void ItemModel::RemoveItem (const QModelIndex& index)
{
	qDebug () << Q_FUNC_INFO << index.isValid () << index.row () << rowCount ();
	if (!index.isValid () || index.row () >= rowCount ())
		return;

	beginRemoveRows (QModelIndex (), index.row (), index.row ());
	Items_.erase (Items_.begin () + index.row ());
	endRemoveRows ();
}

void ItemModel::Activated (const QModelIndex& index) const
{
	if (!index.isValid () || index.row () >= rowCount ())
		return;

    QDesktopServices::openUrl (QUrl (Items_ [index.row ()]->Link_));
}

QString ItemModel::GetDescription (const QModelIndex& index) const
{
    if (!index.isValid () || index.row () >= rowCount ())
        return QString ();

    return Items_ [index.row ()]->Description_;
}

int ItemModel::columnCount (const QModelIndex& parent) const
{
	return ItemHeaders_.size ();
}

QVariant ItemModel::data (const QModelIndex& index, int role) const
{
    if (!index.isValid () || index.row () >= rowCount () || role != Qt::DisplayRole)
        return QVariant ();

	switch (index.column ())
	{
		case 0:
			return Items_ [index.row ()]->Title_;
		default:
			return QVariant ();
	}
}

Qt::ItemFlags ItemModel::flags (const QModelIndex& index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool ItemModel::hasChildren (const QModelIndex& index) const
{
    return !index.isValid ();
}

QVariant ItemModel::headerData (int column, Qt::Orientation orient, int role) const
{
    if (orient == Qt::Horizontal && role == Qt::DisplayRole)
        return ItemHeaders_.at (column);
    else
        return QVariant ();
}

QModelIndex ItemModel::index (int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex (row, column, parent))
        return QModelIndex ();

    return createIndex (row, column);
}

QModelIndex ItemModel::parent (const QModelIndex& index) const
{
    return QModelIndex ();
}

int ItemModel::rowCount (const QModelIndex& parent) const
{
	return Items_.size ();
}

