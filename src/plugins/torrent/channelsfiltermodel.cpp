#include <QStringList>
#include <QtDebug>
#include "channelsfiltermodel.h"
#include "core.h"

ChannelsFilterModel::ChannelsFilterModel (QObject *parent)
: QSortFilterProxyModel (parent)
, NormalMode_ (true)
{
}

void ChannelsFilterModel::setTagsMode ()
{
	NormalMode_ = false;
	invalidateFilter ();
}

void ChannelsFilterModel::setNormalMode ()
{
	NormalMode_ = true;
	invalidateFilter ();
}

bool ChannelsFilterModel::filterAcceptsRow (int source_row, const QModelIndex& source_parent) const
{
	if (NormalMode_)
		return QSortFilterProxyModel::filterAcceptsRow (source_row, source_parent);
	else
	{
		QStringList itemTags = Core::Instance ()->GetTagsForIndex (source_row),
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

