#include "filtermodel.h"
#include <QStringList>

FilterModel::FilterModel (QObject *parent)
: TagsFilterModel (parent)
{
}

FilterModel::~FilterModel ()
{
}

QStringList FilterModel::GetTagsForIndex (int row) const
{
	return sourceModel ()->data (sourceModel ()->index (row, 0),
			TagsRole).toStringList ();
}

