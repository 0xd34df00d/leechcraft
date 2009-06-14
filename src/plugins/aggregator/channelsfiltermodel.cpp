#include <QStringList>
#include <QtDebug>
#include "channelsfiltermodel.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			ChannelsFilterModel::ChannelsFilterModel (QObject *parent)
			: LeechCraft::Util::TagsFilterModel (parent)
			{
				setDynamicSortFilter (true);
				setTagsMode (true);
			}
			
			QStringList ChannelsFilterModel::GetTagsForIndex (int row) const
			{
				return Core::Instance ().GetTagsForIndex (row);
			}
		};
	};
};

