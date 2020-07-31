/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AGGREGATOR_CHANNELSFILTERMODEL_H
#define PLUGINS_AGGREGATOR_CHANNELSFILTERMODEL_H
#include <util/tags/tagsfiltermodel.h>

namespace LC
{
namespace Aggregator
{
	class ChannelsFilterModel : public Util::TagsFilterModel
	{
		Q_OBJECT
	public:
		ChannelsFilterModel (QObject *parent = 0);
	protected:
		virtual QStringList GetTagsForIndex (int) const;
	};
}
}

#endif
