/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagsfoldersproxymodel.h"
#include <util/sll/qtutil.h>
#include <interfaces/structures.h>
#include <interfaces/core/itagsmanager.h>
#include "emitters/tagsmanager.h"

namespace LC::Util
{
	namespace
	{
		FlatToFoldersProxyModel::Config MakeConfig (const ITagsManager& tm)
		{
			return
			{
				.KeysOf_ = [&tm] (const QModelIndex& index)
				{
					return tm.GetTags (index.data (+CustomDataRoles::Tags).value<QList<TagId>> ());
				},
				.UnkeyedLabel_ = TagsFoldersProxyModel::tr ("untagged"),
				.DropFormat_ = "x-leechcraft/tag"_qs,
			};
		}
	}

	TagsFoldersProxyModel::TagsFoldersProxyModel (const ITagsManager& tm, QObject *parent)
	: FlatToFoldersProxyModel { MakeConfig (tm), parent }
	{
		connect (&tm.GetEmitter (),
				&Emitters::TagsManager::nameReplaced,
				this,
				&FlatToFoldersProxyModel::ReplaceKey);
	}
}
