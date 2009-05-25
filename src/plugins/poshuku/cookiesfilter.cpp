#include "cookiesfilter.h"

CookiesFilter::CookiesFilter (QObject *parent)
: QSortFilterProxyModel (parent)
{
}

bool CookiesFilter::filterAcceptsRow (int row, const QModelIndex& parent) const
{
	return parent.isValid () ? true : QSortFilterProxyModel::filterAcceptsRow (row, parent);
}

