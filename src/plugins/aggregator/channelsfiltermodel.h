/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/tags/tagsfiltermodel.h>

namespace LC::Aggregator
{
	class ChannelsFilterModel final : public Util::TagsFilterModel
	{
	public:
		explicit ChannelsFilterModel (QObject *parent = nullptr);
	protected:
		QStringList GetTagsForIndex (int) const override;
	};
}
