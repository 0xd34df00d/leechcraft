#include <algorithm>
#include <functional>
#include "regexpmatchermanager.h"

RegexpMatcherManager::RegexpMatcherManager ()
{
	ItemHeaders_ << tr ("Title matcher") << tr ("Body extractor");
}

RegexpMatcherManager& RegexpMatcherManager::Instance ()
{
	static RegexpMatcherManager inst;
	return inst;
}

RegexpMatcherManager::~RegexpMatcherManager ()
{
}

struct IsEqual : public std::unary_function<RegexpMatcherManager::Item, bool>
{
	QString String_;

	IsEqual (const QString& str)
	: String_ (str)
	{
	}

	bool operator() (const RegexpMatcherManager::Item& it)
	{
		return it.IsEqual (String_);
	}
};

void RegexpMatcherManager::Add (const QString& title, const QString& body)
{
	items_t::const_iterator found = std::find_if (Items_.begin (), Items_.end (), IsEqual (title));
	if (found != Items_.end ())
		throw AlreadyExists ("Regexp user tries to add already exists in the RegexpMatcherManager");

	Item item (title, body);
	beginInsertRows (QModelIndex (), rowCount (), rowCount ());
	Items_.push_back (item);
	endInsertRows ();
}

int RegexpMatcherManager::columnCount (const QModelIndex& parent) const
{
	return ItemHeaders_.size ();
}

QVariant RegexpMatcherManager::data (const QModelIndex& index, int role) const
{
    if (!index.isValid () || index.row () >= rowCount () || role != Qt::DisplayRole)
        return QVariant ();

	items_t::const_iterator pointer = Items_.begin ();
	std::advance (pointer, index.row ());
	switch (index.column ())
	{
		case 0:
			return pointer->Title_;
		case 1:
			return pointer->Body_;
		default:
			return QVariant ();
	}
}

Qt::ItemFlags RegexpMatcherManager::flags (const QModelIndex& index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant RegexpMatcherManager::headerData (int column, Qt::Orientation orient, int role) const
{
    if (orient == Qt::Horizontal && role == Qt::DisplayRole)
        return ItemHeaders_.at (column);
    else
        return QVariant ();
}

QModelIndex RegexpMatcherManager::index (int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex (row, column, parent))
        return QModelIndex ();

    return createIndex (row, column);
}

QModelIndex RegexpMatcherManager::parent (const QModelIndex& index) const
{
    return QModelIndex ();
}

int RegexpMatcherManager::rowCount (const QModelIndex& parent) const
{
	return parent.isValid () ? 0 : Items_.size ();
}
