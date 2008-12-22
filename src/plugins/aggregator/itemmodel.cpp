#include "itemmodel.h"
#include <QSettings>
#include <QUrl>
#include <QTimer>
#include <QVariant>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "item.h"
#include "core.h"

using LeechCraft::Util::Proxy;

ItemModel::ItemModel (QObject *parent)
: QAbstractItemModel (parent)
, SaveScheduled_ (false)
{
	ItemHeaders_ << tr ("Name");

    QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_Aggregator");
    int numItems = settings.beginReadArray ("ItemBucket");
    for (int i = 0; i < numItems; ++i)
    {
        settings.setArrayIndex (i);
		Items_.push_back (Item_ptr (new Item (settings.value ("Item").value<Item> ())));
    }
    settings.endArray ();
}

ItemModel::~ItemModel ()
{
}

void ItemModel::AddItem (const Item_ptr& item)
{
	Item_ptr local (new Item (*item));
	local->Unread_ = false;
	beginInsertRows (QModelIndex (), rowCount (), rowCount ());
	Items_.push_front (local);
	endInsertRows ();

	ScheduleSave ();
}

void ItemModel::RemoveItem (const QModelIndex& index)
{
	if (!index.isValid () || index.row () >= rowCount ())
		return;

	beginRemoveRows (QModelIndex (), index.row (), index.row ());
	Items_.erase (Items_.begin () + index.row ());
	endRemoveRows ();

	ScheduleSave ();
}

void ItemModel::Activated (const QModelIndex& index) const
{
	if (!index.isValid () || index.row () >= rowCount ())
		return;

	Core::Instance ().openLink (Items_ [index.row ()]->Link_);
}

QString ItemModel::GetDescription (const QModelIndex& index) const
{
    if (!index.isValid () || index.row () >= rowCount ())
        return QString ();

    return Items_ [index.row ()]->Description_;
}

int ItemModel::columnCount (const QModelIndex&) const
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

Qt::ItemFlags ItemModel::flags (const QModelIndex&) const
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

QModelIndex ItemModel::parent (const QModelIndex&) const
{
    return QModelIndex ();
}

int ItemModel::rowCount (const QModelIndex&) const
{
	return Items_.size ();
}

void ItemModel::ScheduleSave ()
{
	if (SaveScheduled_)
		return;
	QTimer::singleShot (500, this, SLOT (saveSettings ()));
}

void ItemModel::saveSettings ()
{
    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName () + "_Aggregator");
    settings.beginWriteArray ("ItemBucket");
    settings.remove ("");
    for (size_t i = 0; i < Items_.size (); ++i)
    {
        settings.setArrayIndex (i);
        settings.setValue ("Item", qVariantFromValue<Item> (*Items_ [i]));
    }
    settings.endArray ();
    SaveScheduled_ = false;
}

