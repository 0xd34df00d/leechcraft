/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "todosfproxymodel.h"
#include <util/sll/prelude.h>
#include <interfaces/core/itagsmanager.h>
#include "core.h"
#include "storagemodel.h"

namespace LC
{
namespace Otlozhu
{
	QStringList TodoSFProxyModel::GetTagsForIndex (int row) const
	{
		if (!sourceModel ())
			return QStringList ();

		const auto& ids = sourceModel ()->index (row, 0)
				.data (StorageModel::Roles::ItemTags).toStringList ();
		const auto tm = Core::Instance ().GetProxy ()->GetTagsManager ();
		return Util::Map (ids, [tm] (const QString& id) { return tm->GetTag (id); });
	}

	bool TodoSFProxyModel::lessThan (const QModelIndex& left, const QModelIndex& right) const
	{
		const int leftProg = left.data (StorageModel::Roles::ItemProgress).toInt ();
		const int rightProg = right.data (StorageModel::Roles::ItemProgress).toInt ();
		if (leftProg == 100 && rightProg != 100)
			return true;
		else if (rightProg == 100 && leftProg != 100)
			return false;
		
		return QSortFilterProxyModel::lessThan (left, right);
	}
}
}
