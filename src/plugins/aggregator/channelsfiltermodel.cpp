/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "channelsfiltermodel.h"
#include "common.h"

namespace LC
{
namespace Aggregator
{
	ChannelsFilterModel::ChannelsFilterModel (QObject *parent)
	: Util::TagsFilterModel (parent)
	{
		setDynamicSortFilter (true);
		SetTagsMode (true);
	}
	
	QStringList ChannelsFilterModel::GetTagsForIndex (int row) const
	{
		return sourceModel ()->index (row, 0).data (ChannelRoles::HumanReadableTags).toStringList ();
	}
}
}
