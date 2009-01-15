#include "urlcompletionmodel.h"
#include <QUrl>
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

	int realIndex = index.row () / 3;
	int shift = index.row () % 3;
	if (role == Qt::DisplayRole)
	{
		Populate ();
		return Items_ [realIndex].Title_ + " [" + Items_ [realIndex].URL_ + "]";
	}
	else if (role == Qt::EditRole)
	{
		Populate ();
		QString origURL = Items_ [realIndex].URL_;
		if (shift == 0)
			return origURL;
		else if (shift == 1)
		{
			QUrl url (origURL);
			return origURL.right (origURL.size () - url.scheme ().size () - 3);
		}
		else if (shift == 2)
		{
			QUrl url (origURL);
			QString newURL = origURL.right (origURL.size () - url.scheme ().size () - 3);
			if (newURL.startsWith ("www."))
				newURL = newURL.right (newURL.size () - 4);
			return newURL;
		}
	}
	else
		return QVariant ();
}

Qt::ItemFlags URLCompletionModel::flags (const QModelIndex&) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant URLCompletionModel::headerData (int, Qt::Orientation, int) const
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
		return Items_.size () * 3;
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

