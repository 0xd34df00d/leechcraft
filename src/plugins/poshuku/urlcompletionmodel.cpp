#include "urlcompletionmodel.h"
#include "core.h"

URLCompletionModel::URLCompletionModel (QObject *parent)
: QAbstractItemModel (parent)
, Valid_ (false)
{
}

URLCompletionModel::~URLCompletionModel ()
{
}

int URLCompletionModel::columnCount (const QModelIndex&) const
{
	return 1;
}

QVariant URLCompletionModel::data (const QModelIndex& index, int role) const
{
	if (!index.isValid ())
		return QVariant ();

	if (role == Qt::DisplayRole)
	{
		Populate ();
		return Items_ [index.row ()].Title_ + " [" + Items_ [index.row ()].URL_ + "]";
	}
	else if (role == Qt::EditRole)
	{
		Populate ();
		return Items_ [index.row ()].URL_;
	}
	else
		return QVariant ();
}

Qt::ItemFlags URLCompletionModel::flags (const QModelIndex&) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant URLCompletionModel::headerData (int column, Qt::Orientation orient,
		int role) const
{
	return QVariant ();
}

QModelIndex URLCompletionModel::index (int row, int column,
		const QModelIndex& parent) const
{
	if (!hasIndex (row, column, parent))
		return QModelIndex ();

	return createIndex (row, column);
}

QModelIndex URLCompletionModel::parent (const QModelIndex&) const
{
	return QModelIndex ();
}

int URLCompletionModel::rowCount (const QModelIndex& index) const
{
	if (index.isValid ())
		return 0;
	else
	{
		Populate ();
		return Items_.size ();
	}
}

void URLCompletionModel::handleItemAdded (const HistoryModel::HistoryItem&)
{
	Valid_ = false;
}

void URLCompletionModel::Populate () const
{
	if (!Valid_)
	{
		Items_.clear ();
		Core::Instance ().GetStorageBackend ()->LoadUniqueHistory (Items_);
		Valid_ = true;
	}
}

