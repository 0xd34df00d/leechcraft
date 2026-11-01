/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/models/flattofoldersproxymodel.h>
#include "tagsconfig.h"

class ITagsManager;

namespace LC::Util
{
	/** @brief Groups the rows of a flat source model into folders by their tags.
	 *
	 * The tags of a row are read from its column 0 via `CustomDataRoles::Tags`
	 * as a `QList<TagId>` and resolved through the tags manager, so the row
	 * appears under the folder of every name its tags have, and the rows
	 * whose tags have no names go into the untagged folder. A drop onto a
	 * folder carries the folder's name under the `x-leechcraft/tag` format,
	 * empty for the untagged folder. The folders follow the name replacements
	 * the tags manager announces.
	 *
	 * The tags manager shall outlive the model.
	 *
	 * @sa FlatToFoldersProxyModel
	 */
	class UTIL_TAGS_API TagsFoldersProxyModel final : public FlatToFoldersProxyModel
	{
		Q_OBJECT
	public:
		explicit TagsFoldersProxyModel (const ITagsManager&, QObject* = nullptr);
	};
}
