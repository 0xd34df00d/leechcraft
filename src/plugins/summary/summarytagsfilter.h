/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/tags/tagsfiltermodel.h>

namespace LC
{
namespace Summary
{
	class SummaryTagsFilter : public Util::TagsFilterModel
	{
		Q_OBJECT
	public:
		SummaryTagsFilter (QObject* = 0);

		QVariant data (const QModelIndex&, int) const override;
	protected:
		QStringList GetTagsForIndex (int) const override;
	};
}
}
