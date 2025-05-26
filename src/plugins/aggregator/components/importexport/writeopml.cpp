/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "writeopml.h"
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/sll/xmlnode.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "components/storage/storagebackendmanager.h"

namespace LC::Aggregator::OPML
{
	using Util::Tag;

	namespace
	{
		Tag MakeHead (const QString& title, const QString& owner, const QString& ownerEmail)
		{
			return
			{
				.Name_ = "head"_qba,
				.Children_ =
				{
					Tag::WithTextNonEmpty ("title"_qba, title),
					Tag::WithTextNonEmpty ("ownerName"_qba, owner),
					Tag::WithTextNonEmpty ("ownerEmail"_qba, ownerEmail),
				}
			};
		}

		Tag MakeBody (const channels_shorts_t& channels)
		{
			const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
			const auto itm = GetProxyHolder ()->GetTagsManager ();

			return
			{
				.Name_ = "body"_qba,
				.Children_ = Util::MapAs<QList> (channels,
						[&] (const ChannelShort& cs) -> Util::Node
						{
							const auto& tags = itm->GetTags (cs.Tags_).join (',');
							return Tag
							{
								.Name_ = "outline"_qba,
								.Attrs_ = {
									{ "isOpen"_qba, "true"_qs },
									{ "title"_qba, cs.Title_ },
									{ "htmlUrl"_qba, cs.Link_ },
									{ "xmlUrl"_qba, sb->GetFeed (cs.FeedID_).URL_ },
									{ "category"_qba, tags },
								}
							};
						})
			};
		}
	}

	QByteArray Write (const channels_shorts_t& channels,
			const QString& title,
			const QString& owner,
			const QString& ownerEmail)
	{
		return Tag
		{
			.Name_ = "opml"_qba,
			.Children_ = { MakeHead (title, owner, ownerEmail), MakeBody (channels), },
		}.Serialize<QByteArray> ({ .Prolog_ = true, .Indent_ = 2 });
	}
}
