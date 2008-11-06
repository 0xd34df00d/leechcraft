#include <QStringList>
#include <QtDebug>
#include "channelsfiltermodel.h"
#include "core.h"

ChannelsFilterModel::ChannelsFilterModel (QObject *parent)
: QSortFilterProxyModel (parent)
{
}

bool ChannelsFilterModel::filterAcceptsRow (int source_row, const QModelIndex&) const
{
    QStringList itemTags = Core::Instance ().GetTagsForIndex (source_row),
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

