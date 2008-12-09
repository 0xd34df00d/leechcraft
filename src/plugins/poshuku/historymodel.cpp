#include "historymodel.h"
#include <QSettings>
#include <QTimer>
#include <plugininterface/proxy.h>
#include "core.h"

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

void HistoryModel::handleItemAdded (const HistoryModel::HistoryItem& item)
{
	beginInsertRows (QModelIndex (), 0, 0);
	Items_.push_front (item);
	endInsertRows ();
}

