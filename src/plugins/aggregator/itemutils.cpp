/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemutils.h"

namespace LC::Aggregator::ItemUtils
{
	QSet<QString> GetCategories (const items_shorts_t& items)
	{
		QSet<QString> unique;
		for (const auto& item : items)
			for (const auto& category : item.Categories_)
				unique << category;

		return unique;
	}
}
