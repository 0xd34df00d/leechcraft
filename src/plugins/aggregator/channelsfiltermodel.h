#ifndef PLUGINS_AGGREGATOR_CHANNELSFILTERMODEL_H
#define PLUGINS_AGGREGATOR_CHANNELSFILTERMODEL_H
#include <plugininterface/tagsfiltermodel.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class ChannelsFilterModel : public LeechCraft::Util::TagsFilterModel
			{
			    Q_OBJECT
			public:
			    ChannelsFilterModel (QObject *parent = 0);
			protected:
				virtual QStringList GetTagsForIndex (int) const;
			};
		};
	};
};

#endif

