#include "historymodel.h"
#include <algorithm>
#include <QTimer>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "core.h"
#include "xmlsettingsmanager.h"

HistoryModel::HistoryModel (QObject *parent)
: QAbstractItemModel (parent)
{
	ItemHeaders_ << tr ("Title")
		<< tr ("Date")
		<< tr ("URL");
	QTimer::singleShot (0, this, SLOT (loadData ()));
}

HistoryModel::~HistoryModel ()
{
}

int HistoryModel::columnCount (const QModelIndex&) const
{
	return ItemHeaders_.size ();
}

QVariant HistoryModel::data (const QModelIndex& index, int role) const
{
	if (!index.isValid ())
		return QVariant ();

	switch (role)
	{
		case Qt::DisplayRole:
			switch (index.column ())
			{
				case ColumnTitle:
					return Items_ [index.row ()].Title_;
				case ColumnDate:
					return Items_ [index.row ()].DateTime_.toString ();
				case ColumnURL:
					return Items_ [index.row ()].URL_;
				default:
					return QVariant ();
			}
		case CompletionRole:
			return Items_ [index.row ()].URL_;
		default:
			return QVariant ();
	}
}

Qt::ItemFlags HistoryModel::flags (const QModelIndex&) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant HistoryModel::headerData (int column, Qt::Orientation orient,
		int role) const
{
	if (orient == Qt::Horizontal && role == Qt::DisplayRole)
		return ItemHeaders_.at (column);
	else
		return QVariant ();
}

QModelIndex HistoryModel::index (int row, int column,
		const QModelIndex& parent) const
{
	if (!hasIndex (row, column, parent))
		return QModelIndex ();

	return createIndex (row, column);
}

QModelIndex HistoryModel::parent (const QModelIndex&) const
{
	return QModelIndex ();
}

int HistoryModel::rowCount (const QModelIndex& index) const
{
	return index.isValid () ? 0 : Items_.size ();
}

namespace
{
	struct HistoryEraser
	{
		int Age_;

		HistoryEraser (int age)
		: Age_ (age)
		{
		}

		bool operator() (const HistoryItem& j, const QDateTime& current)
		{
			return j.DateTime_.daysTo (current) < Age_;
		}
	};
};

void HistoryModel::AddItem (const QString& title, const QString& url,
		const QDateTime& date)
{
	HistoryItem item =
	{
		title,
		date,
		url
	};
	Core::Instance ().GetStorageBackend ()->AddToHistory (item);

	int age = XmlSettingsManager::Instance ()->
				property ("HistoryClearOlderThan").toInt ();

	Core::Instance ().GetStorageBackend ()->ClearOldHistory (age);

	std::deque<HistoryItem>::iterator pos =
		std::lower_bound (Items_.begin (), Items_.end (),
				QDateTime::currentDateTime (), HistoryEraser (age));

	if (pos == Items_.end ())
		return;

	int index = std::distance (Items_.begin (), pos);
	beginRemoveRows (QModelIndex (), index, Items_.size () - 1);
	Items_.erase (pos, Items_.end ());
	endRemoveRows ();
}

void HistoryModel::loadData ()
{
	std::vector<HistoryItem> items;
	Core::Instance ().GetStorageBackend ()->LoadHistory (items);

	if (!items.size ())
		return;

	beginInsertRows (QModelIndex (), 0, items.size () - 1);
	for (std::vector<HistoryItem>::const_reverse_iterator i = items.rbegin (),
			end = items.rend (); i != end; ++i)
		Items_.push_front (*i);
	endInsertRows ();
}

void HistoryModel::handleItemAdded (const HistoryItem& item)
{
	beginInsertRows (QModelIndex (), 0, 0);
	Items_.push_front (item);
	endInsertRows ();
}

