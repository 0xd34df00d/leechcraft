#include <QStringList>
#include <QtDebug>
#include "channelsfiltermodel.h"
#include "core.h"

ChannelsFilterModel::ChannelsFilterModel (QObject *parent)
: TagsFilterModel (parent)
{
	setTagsMode (true);
}

QStringList ChannelsFilterModel::GetTagsForIndex (int row) const
{
	return Core::Instance ().GetTagsForIndex (row);
}

