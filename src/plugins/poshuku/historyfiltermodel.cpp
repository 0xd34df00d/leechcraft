#include "historyfiltermodel.h"

HistoryFilterModel::HistoryFilterModel (QObject *parent)
: QSortFilterProxyModel (parent)
{
}

bool HistoryFilterModel::filterAcceptsRow (int row, const QModelIndex& parent) const
{
	if (sourceModel ()->rowCount (sourceModel ()->index (row, 0, parent)))
		return true;
	else
		return QSortFilterProxyModel::filterAcceptsRow (row, parent);
}

