#ifndef FILTERMODEL_H
#define FILTERMODEL_H
#include <plugininterface/tagsfiltermodel.h>

class FilterModel : public TagsFilterModel
{
	Q_OBJECT
public:
	enum { TagsRole = 42 };
	FilterModel (QObject* = 0);
	virtual ~FilterModel ();
protected:
	virtual QStringList GetTagsForIndex (int) const;
};

#endif

