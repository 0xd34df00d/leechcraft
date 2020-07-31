/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "similarmodel.h"
#include <interfaces/media/audiostructs.h>
#include "core.h"
#include "localcollection.h"

namespace LC
{
namespace LMP
{
	SimilarModel::SimilarModel (QObject *parent)
	: RoleNamesMixin<QStandardItemModel> (parent)
	{
		QHash<int, QByteArray> names;
		names [ArtistName] = "artistName";
		names [Similarity] = "similarity";
		names [ArtistImageURL] = "artistImageURL";
		names [ArtistBigImageURL] = "artistBigImageURL";
		names [ArtistPageURL] = "artistPageURL";
		names [ArtistTags] = "artistTags";
		names [ShortDesc] = "shortDesc";
		names [FullDesc] = "fullDesc";
		names [IsInCollection] = "artistInCollection";
		setRoleNames (names);
	}

	QStandardItem* SimilarModel::ConstructItem (const Media::ArtistInfo& artist)
	{
		auto item = new QStandardItem;
		item->setData (artist.Name_, SimilarModel::Role::ArtistName);
		item->setData (artist.Image_, SimilarModel::Role::ArtistImageURL);
		item->setData (artist.LargeImage_, SimilarModel::Role::ArtistBigImageURL);
		item->setData (artist.ShortDesc_, SimilarModel::Role::ShortDesc);
		item->setData (artist.FullDesc_, SimilarModel::Role::FullDesc);
		item->setData (artist.Page_, SimilarModel::Role::ArtistPageURL);

		QStringList tags;
		const int diff = artist.Tags_.size () - 5;
		auto begin = artist.Tags_.begin ();
		if (diff > 0)
			std::advance (begin, diff);
		std::transform (begin, artist.Tags_.end (), std::back_inserter (tags),
				[] (const auto& tag) { return tag.Name_; });
		std::reverse (tags.begin (), tags.end ());
		item->setData (tr ("Tags: %1").arg (tags.join ("; ")), SimilarModel::Role::ArtistTags);

		const auto col = Core::Instance ().GetLocalCollection ();
		item->setData (col->FindArtist (artist.Name_) >= 0, SimilarModel::Role::IsInCollection);

		return item;
	}
}
}
