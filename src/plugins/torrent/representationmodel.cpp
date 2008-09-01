#include "representationmodel.h"

RepresentationModel::RepresentationModel (QObject *parent)
: QSortFilterProxyModel (parent)
{
}

RepresentationModel::~RepresentationModel ()
{
}

bool RepresentationModel::filterAcceptsColumn (int column, const QModelIndex& parent) const
{
	if (column > 3)
		return false;
	else
		return true;
}

