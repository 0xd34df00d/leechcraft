/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "similarmodel.h"
#include <interfaces/media/audiostructs.h>
#include "core.h"
#include "localcollection.h"

namespace LeechCraft
{
namespace LMP
{
	SimilarModel::SimilarModel (QObject *parent)
	: QStandardItemModel (parent)
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
				[] (decltype (artist.Tags_.front ()) tag) { return tag.Name_; });
		std::reverse (tags.begin (), tags.end ());
		item->setData (tr ("Tags: %1").arg (tags.join ("; ")), SimilarModel::Role::ArtistTags);

		const auto col = Core::Instance ().GetLocalCollection ();
		item->setData (col->FindArtist (artist.Name_) >= 0, SimilarModel::Role::IsInCollection);

		return item;
	}
}
}
