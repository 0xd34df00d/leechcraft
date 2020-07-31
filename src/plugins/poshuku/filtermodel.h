/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_FILTERMODEL_H
#define PLUGINS_POSHUKU_FILTERMODEL_H
#include <util/tags/tagsfiltermodel.h>

namespace LC
{
namespace Poshuku
{
	class FilterModel : public Util::TagsFilterModel
	{
	public:
		using Util::TagsFilterModel::TagsFilterModel;
	protected:
		QStringList GetTagsForIndex (int) const override;
	};
}
}

#endif
