#ifndef CHANNELSFILTERMODEL_H
#define CHANNELSFILTERMODEL_H
#include <plugininterface/tagsfiltermodel.h>

class ChannelsFilterModel : public TagsFilterModel
{
    Q_OBJECT
public:
    ChannelsFilterModel (QObject *parent = 0);
protected:
	virtual QStringList GetTagsForIndex (int) const;
};

#endif

