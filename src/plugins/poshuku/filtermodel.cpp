/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filtermodel.h"
#include <QStringList>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "favoritesmodel.h"
#include "core.h"

namespace LC
{
namespace Poshuku
{
	QStringList FilterModel::GetTagsForIndex (int row) const
	{
		const auto& ids = sourceModel ()->data (sourceModel ()->index (row, 0), RoleTags).toStringList ();
		return Core::Instance ().GetProxy ()->GetTagsManager ()->GetTags (ids);
	}
}
}
