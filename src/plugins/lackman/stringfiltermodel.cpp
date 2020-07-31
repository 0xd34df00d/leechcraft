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
	: QSortFilterProxyModel (parent)
	{
	}

	bool StringFilterModel::filterAcceptsRow (int sourceRow, const QModelIndex& sourceParent) const
	{
		if (QSortFilterProxyModel::filterAcceptsRow (sourceRow, sourceParent))
			return true;

		const QString& filterString = filterRegExp ().pattern ();
		const QModelIndex& idx = sourceModel ()->index (sourceRow, 0, sourceParent);

		if (sourceModel ()->data (idx, PackagesModel::PMRShortDescription)
				.toString ().contains (filterString, Qt::CaseInsensitive))
			return true;

		const auto& tags = Util::AsSet (sourceModel ()->data (idx, PackagesModel::PMRTags).toStringList ());
		const auto& queryList = Core::Instance ().GetProxy ()->GetTagsManager ()->Split (filterString);
		auto userDefined = Util::AsSet (queryList);

		return tags.contains (userDefined);
	}
}
}
