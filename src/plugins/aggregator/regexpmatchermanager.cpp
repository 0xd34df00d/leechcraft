#include "regexpmatchermanager.h"
#include <algorithm>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <QTimer>
#include <QtDebug>
#include <QSettings>
#include <plugininterface/proxy.h>
#include <plugininterface/util.h>
#include "item.h"

using LeechCraft::Util::Proxy;

RegexpMatcherManager::RegexpItem::RegexpItem (const QString& title,
		const QString& body)
: Title_ (title)
, Body_ (body)
{
}

bool RegexpMatcherManager::RegexpItem::operator== (const RegexpMatcherManager::RegexpItem& other) const
{
	return Title_ == other.Title_ &&
		Body_ == other.Body_;
}

bool RegexpMatcherManager::RegexpItem::IsEqual (const QString& str) const
{
	return Title_ == str;
}

QByteArray RegexpMatcherManager::RegexpItem::Serialize () const
{
	QByteArray result;
	{
		QDataStream out (&result, QIODevice::WriteOnly);
		int version = 1;
		out << version << Title_ << Body_;
	}
	return result;
}

void RegexpMatcherManager::RegexpItem::Deserialize (const QByteArray& data)
{
	QDataStream in (data);
	int version = 0;
	in >> version;
	if (version == 1)
		in >> Title_ >> Body_;
	else
		throw std::runtime_error ("Unknown version");
}

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
struct IsEqual : public std::unary_function<RegexpMatcherManager::RegexpItem, bool>
{
	QString String_;

	IsEqual (const QString& str)
	: String_ (str)
	{
	}

	bool operator() (const RegexpMatcherManager::RegexpItem& it)
	{
		return it.IsEqual (String_);
	}
};

namespace
{
	inline bool IsRegexpValid (const QString& rx)
	{
		return QRegExp (rx).isValid ();
	}
};

void RegexpMatcherManager::Add (const QString& title, const QString& body)
{
	if (!IsRegexpValid (title) || !IsRegexpValid (body))
		throw Malformed ("Regexp is malformed");

	items_t::const_iterator found = std::find_if (Items_.begin (), Items_.end (), IsEqual (title));
	if (found != Items_.end ())
		throw AlreadyExists ("Regexp user tries to add already exists in the RegexpMatcherManager");

	beginInsertRows (QModelIndex (), rowCount (), rowCount ());
	Items_.push_back (RegexpItem (title, body));
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
	if (index.row () >= static_cast<int> (Items_.size ()))
		return result;

	result.first = Items_ [index.row ()].Title_;
	result.second = Items_ [index.row ()].Body_;
	return result;
}

namespace
{
	struct IfTitleMatches : public std::unary_function<RegexpMatcherManager::RegexpItem, bool>
	{
		QString Title_;

		IfTitleMatches (const QString& str)
		: Title_ (str)
		{
		}

		bool operator() (const RegexpMatcherManager::RegexpItem& item)
		{
			return QRegExp (item.Title_).exactMatch (Title_);
		}
	};

	struct HandleBody : public std::unary_function<RegexpMatcherManager::RegexpItem, void>
	{
		QString Link_, Description_;
		QStringList Links_;

		HandleBody (const QString& descr, const QString& link)
		: Link_ (link)
		, Description_ (descr)
		{
		}

		void operator() (const RegexpMatcherManager::RegexpItem& item)
		{
			QString rxs = item.Body_;
			bool link = false;
			if (rxs.startsWith ("\\link"))
			{
				rxs = rxs.right (rxs.size () - 5);
				link = true;
			}

			QRegExp ib (rxs, Qt::CaseInsensitive, QRegExp::RegExp2);
			if (rxs.isEmpty () && !Link_.isEmpty ())
				Links_ << Link_;
			else if ((!link && ib.indexIn (Description_) != -1) ||
					(link && ib.indexIn (Link_) != -1))
				Links_ << ib.cap (0);
		}

		QStringList GetLinks ()
		{
			std::sort (Links_.begin (), Links_.end ());

			QStringList result;
			std::unique_copy (Links_.begin (),
					Links_.end (),
					std::back_inserter (result));
			return result;
		}
	};
};

void RegexpMatcherManager::HandleItem (const Item_ptr& item) const
{
	std::deque<RegexpItem> matchingTitles;

	LeechCraft::Util::copy_if (Items_.begin (), Items_.end (),
			std::back_inserter (matchingTitles),
			IfTitleMatches (item->Title_));

	QStringList links = std::for_each (matchingTitles.begin (),
			matchingTitles.end (),
			HandleBody (item->Description_, item->Link_)).GetLinks ();

	for (QStringList::const_iterator i = links.begin (),
			end = links.end ();	i != end; ++i)
		emit gotLink (i->toUtf8 ());
}

int RegexpMatcherManager::columnCount (const QModelIndex&) const
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

Qt::ItemFlags RegexpMatcherManager::flags (const QModelIndex&) const
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

QModelIndex RegexpMatcherManager::parent (const QModelIndex&) const
{
    return QModelIndex ();
}

int RegexpMatcherManager::rowCount (const QModelIndex& parent) const
{
	return parent.isValid () ? 0 : Items_.size ();
}

namespace
{
	struct WriteOut
	{
		QSettings& Settings_;
		int Index_;

		WriteOut (QSettings& settings)
		: Settings_ (settings)
		, Index_ (0)
		{
		}

		void operator() (const RegexpMatcherManager::RegexpItem& item)
		{
			Settings_.setArrayIndex (Index_++);
			Settings_.setValue ("Item", item.Serialize ());
		}
	};
};

void RegexpMatcherManager::saveSettings () const
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_Aggregator");
	settings.beginWriteArray ("RegexpMatcher");
	settings.remove ("");
	std::for_each (Items_.begin (), Items_.end (), WriteOut (settings));
	settings.endArray ();

	SaveScheduled_ = false;
}

void RegexpMatcherManager::RestoreSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_Aggregator");
	int size = settings.beginReadArray ("RegexpMatcher");
	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex (i);
		QByteArray data = settings.value ("Item").toByteArray ();
		RegexpItem item;
		try
		{
			item.Deserialize (data);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO << e.what ();
			continue;
		}
		Items_.push_back (item);
	}
	settings.endArray ();
}

void RegexpMatcherManager::ScheduleSave ()
{
	if (SaveScheduled_)
		return;

	QTimer::singleShot (100, this, SLOT (saveSettings ()));
	SaveScheduled_ = true;
}

