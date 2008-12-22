#include <QStringList>
#include <QtDebug>
#include "tagsfiltermodel.h"
#include "core.h"

using namespace LeechCraft::Util;

TagsFilterModel::TagsFilterModel (QObject *parent)
: QSortFilterProxyModel (parent)
, NormalMode_ (true)
{
}

void TagsFilterModel::setTagsMode (bool tags)
{
	NormalMode_ = !tags;
}

bool TagsFilterModel::filterAcceptsRow (int source_row, const QModelIndex& index) const
{
	if (NormalMode_)
		return QSortFilterProxyModel::filterAcceptsRow (source_row, index);
	else
	{
		QStringList itemTags = GetTagsForIndex (source_row),
					filterTags = filterRegExp ().pattern ().split (' ', QString::SkipEmptyParts);
		if (!filterTags.size ())
			return true;

		for (int i = 0; i < filterTags.size (); ++i)
		{
			bool found = false;
			for (int j = 0; j < itemTags.size (); ++j)
				if (itemTags.at (j).contains (filterTags.at (i)))
				{
					found = true;
					break;
				}
			if (!found)
				return false;
		}
		return true;
	}
}

