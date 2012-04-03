/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "todosfproxymodel.h"
#include <algorithm>
#include <interfaces/core/itagsmanager.h>
#include "core.h"
#include "storagemodel.h"

namespace LeechCraft
{
namespace Otlozhu
{
	TodoSFProxyModel::TodoSFProxyModel (QObject *parent)
	: Util::TagsFilterModel (parent)
	{
	}

	QStringList TodoSFProxyModel::GetTagsForIndex (int row) const
	{
		if (!sourceModel ())
			return QStringList ();

		const auto& ids = sourceModel ()->index (row, 0)
				.data (StorageModel::Roles::ItemTags).toStringList ();
		const auto tm = Core::Instance ().GetProxy ()->GetTagsManager ();
		QStringList result;
		std::transform (ids.begin (), ids.end (), std::back_inserter (result),
				[tm] (const QString& id) { return tm->GetTag (id); });
		return result;
	}
}
}
