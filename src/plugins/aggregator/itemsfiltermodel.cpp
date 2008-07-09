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
	if (HideRead_ && Core::Instance ().IsItemRead (sourceRow))
		return false;
	else
		return QSortFilterProxyModel::filterAcceptsRow (sourceRow,
				sourceParent);
}

