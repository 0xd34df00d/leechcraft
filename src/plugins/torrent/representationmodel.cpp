#include "representationmodel.h"

RepresentationModel::RepresentationModel (QObject *parent)
: QSortFilterProxyModel (parent)
{
	setObjectName ("Torrent RepresentationModel");
}

RepresentationModel::~RepresentationModel ()
{
}

