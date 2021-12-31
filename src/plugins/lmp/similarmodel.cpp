/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "similarmodel.h"
#include <util/models/roleditemsmodel.h>
#include <interfaces/media/audiostructs.h>
#include "localcollection.h"

namespace LC::LMP
{
	SimilarArtistInfo::SimilarArtistInfo (const Media::ArtistInfo& artist, const LocalCollection& coll)
	: Media::ArtistInfo { artist }
	, IsInCollection_ { coll.FindArtist (artist.Name_) >= 0 }
	{
		QStringList tags;
		const int diff = artist.Tags_.size () - 5;
		auto begin = artist.Tags_.begin ();
		if (diff > 0)
			std::advance (begin, diff);
		std::transform (begin, artist.Tags_.end (), std::back_inserter (tags),
				[] (const auto& tag) { return tag.Name_; });
		std::reverse (tags.begin (), tags.end ());

		TagsString_ = QObject::tr ("Tags: %1").arg (tags.join ("; "));
	}

	SimilarModel* MakeSimilarModel (QObject *parent)
	{
		using Util::RoledMemberField_v;
		return new SimilarModel
		{
			parent,
			RoledMemberField_v<"artistName", &SimilarArtistInfo::Name_>,
			RoledMemberField_v<"artistImageURL", &SimilarArtistInfo::Image_>,
			RoledMemberField_v<"artistBigImageURL", &SimilarArtistInfo::LargeImage_>,
			RoledMemberField_v<"artistPageURL", &SimilarArtistInfo::Page_>,
			RoledMemberField_v<"artistTags", &SimilarArtistInfo::TagsString_>,
			RoledMemberField_v<"shortDesc", &SimilarArtistInfo::ShortDesc_>,
			RoledMemberField_v<"fullDesc", &SimilarArtistInfo::FullDesc_>,
			RoledMemberField_v<"artistInCollection", &SimilarArtistInfo::IsInCollection_>,
			RoledMemberField_v<"similarity", &SimilarArtistInfo::Similarity_>,
		};
	}
}
