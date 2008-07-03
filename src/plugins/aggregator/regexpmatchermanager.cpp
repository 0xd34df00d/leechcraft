#include <QTimer>
#include <QSettings>
#include <algorithm>
#include <functional>
#include <iterator>
#include <plugininterface/proxy.h>
#include "regexpmatchermanager.h"

RegexpMatcherManager::RegexpMatcherManager ()
: SaveScheduled_ (false)
{
	ItemHeaders_ << tr ("Title matcher") << tr ("Body extractor");
	RestoreSettings ();
}

RegexpMatcherManager& RegexpMatcherManager::Instance ()
{
	static RegexpMatcherManager inst;
	return inst;
}

RegexpMatcherManager::~RegexpMatcherManager ()
{
}

void RegexpMatcherManager::Release ()
{
	saveSettings ();
}

// Finds items with same title
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

namespace
{
	inline bool IsRegexpValid (const QString& rx)
	{
		return QRegExp (rx).isValid () && !QRegExp (rx).isEmpty ();
	}
};

void RegexpMatcherManager::Add (const QString& title, const QString& body)
{
	if (!IsRegexpValid (title) || !IsRegexpValid (body))
		throw Malformed ("Regexp is malformed");

	items_t::const_iterator found = std::find_if (Items_.begin (), Items_.end (), IsEqual (title));
	if (found != Items_.end ())
		throw AlreadyExists ("Regexp user tries to add already exists in the RegexpMatcherManager");

	Item item (title, body);
	beginInsertRows (QModelIndex (), rowCount (), rowCount ());
	Items_.push_back (item);
	endInsertRows ();

	ScheduleSave ();
}

void RegexpMatcherManager::Remove (const QString& title)
{
	items_t::iterator found = std::find_if (Items_.begin (), Items_.end (), IsEqual (title));
	if (found == Items_.end ())
		throw NotFound ("Regexp user tried to remove doesn't exist in the RegexpMatcherManager");

	int dst = std::distance (Items_.begin (), found);
	beginRemoveRows (QModelIndex (), dst, dst);
	Items_.erase (found);
	endRemoveRows ();

	ScheduleSave ();
}

void RegexpMatcherManager::Remove (const QModelIndex& index)
{
	items_t::iterator begin = Items_.begin ();
	std::advance (begin, index.row ());

	beginRemoveRows (QModelIndex (), index.row (), index.row ());
	Items_.erase (begin);
	endRemoveRows ();

	ScheduleSave ();
}

void RegexpMatcherManager::Modify (const QString& title, const QString& newBody)
{
	if (!IsRegexpValid (title) || !IsRegexpValid (newBody))
		throw Malformed ("Regexp is malformed");

	items_t::iterator found = std::find_if (Items_.begin (), Items_.end (), IsEqual (title));
	if (found == Items_.end ())
		throw NotFound ("Regexp user tried to modify doesn't exist in the RegexpMatcherManager");

	found->Body_ = newBody;
	int dst = std::distance (Items_.begin (), found);
	emit dataChanged (index (dst, 1), index (dst, 1));

	ScheduleSave ();
}

RegexpMatcherManager::titlebody_t RegexpMatcherManager::GetTitleBody (const QModelIndex& index) const
{
	titlebody_t result;
	if (index.row () >= Items_.size ())
		return result;

	result.first = Items_ [index.row ()].Title_;
	result.second = Items_ [index.row ()].Body_;
	return result;
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

void RegexpMatcherManager::saveSettings () const
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_Aggregator");

	SaveScheduled_ = false;
}

void RegexpMatcherManager::RestoreSettings ()
{
}

void RegexpMatcherManager::ScheduleSave ()
{
	if (SaveScheduled_)
		return;

	QTimer::singleShot (100, this, SLOT (saveSettings ()));
	SaveScheduled_ = true;
}

