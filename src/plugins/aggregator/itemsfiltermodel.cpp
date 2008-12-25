#include "core.h"
#include "itemsfiltermodel.h"

ItemsFilterModel::ItemsFilterModel (QObject *parent)
: QSortFilterProxyModel (parent)
, HideRead_ (false)
{
}

ItemsFilterModel::~ItemsFilterModel ()
{
}

void ItemsFilterModel::SetHideRead (bool hide)
{
	HideRead_ = hide;
	invalidateFilter ();
}

bool ItemsFilterModel::filterAcceptsRow (int sourceRow,
		const QModelIndex& sourceParent) const
{
	if (!ItemCategories_.isEmpty ())
	{
		bool categoryFound = false;
		QStringList itemCategories =
			Core::Instance ().GetItemCategories (sourceRow);
		for (QStringList::const_iterator i = itemCategories.begin (),
				end = itemCategories.end (); i != end; ++i)
			if (ItemCategories_.contains (*i))
			{
				categoryFound = true;
				break;
			}

		if (!categoryFound)
			return false;
	}

	if (HideRead_ &&
			Core::Instance ().IsItemRead (sourceRow) &&
			!Core::Instance ().IsItemCurrent (sourceRow))
		return false;
	else
		return QSortFilterProxyModel::filterAcceptsRow (sourceRow,
				sourceParent);
}

void ItemsFilterModel::categorySelectionChanged (const QStringList& categories)
{
	ItemCategories_ = categories;
	invalidateFilter ();
}

