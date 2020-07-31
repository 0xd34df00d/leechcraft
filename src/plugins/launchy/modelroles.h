/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <Qt>
#include <QMetaType>

namespace LC
{
namespace Launchy
{
	enum ModelRoles
	{
		CategoryName = Qt::UserRole + 1,
		CategoryIcon,
		CategoryType,

		ItemName,
		ItemIcon,
		ItemDescription,
		ItemID,
		ItemCommand,

		IsItemFavorite,
		IsItemRecent,
		ItemRecentPos,

		ItemNativeCategories,
		NativeCategories,

		ExecutorFunctor
	};

	typedef std::function<void ()> Executor_f;
}
}

Q_DECLARE_METATYPE (LC::Launchy::Executor_f)
