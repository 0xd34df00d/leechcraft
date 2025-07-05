/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "stringfiltermodel.h"
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include <util/sll/containerconversions.h>
#include "packagesmodel.h"
#include "core.h"

namespace LC
{
namespace LackMan
{
	StringFilterModel::StringFilterModel (QObject *parent)
	: FixedStringFilterProxyModel (parent)
	{
		using enum PackagesModel::PackageModelRole;
		SetFilterRoles ({ PMRName, PMRShortDescription, PMRVersion });
	}

	bool StringFilterModel::filterAcceptsRow (int sourceRow, const QModelIndex& sourceParent) const
	{
		if (FixedStringFilterProxyModel::filterAcceptsRow (sourceRow, sourceParent))
			return true;

		const auto& sourceIdx = sourceModel ()->index (sourceRow, 0, sourceParent);
		const auto& itemTags = Util::AsSet (sourceIdx.data (PackagesModel::PMRTags).toStringList ());
		const auto& queryTags = Util::AsSet (Core::Instance ().GetProxy ()->GetTagsManager ()->Split (GetFilterString ()));
		return itemTags.contains (queryTags);
	}
}
}
