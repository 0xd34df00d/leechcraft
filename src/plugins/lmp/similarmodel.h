/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/media/audiostructs.h>

namespace Media
{
	struct ArtistInfo;
}

namespace LC::Util
{
	template<typename>
	class RoledItemsModel;
}

namespace LC::LMP
{
	class LocalCollection;

	struct SimilarArtistInfo : Media::ArtistInfo
	{
		QString TagsString_;
		bool IsInCollection_ = false;
		QString Similarity_ = 0;

		SimilarArtistInfo (const Media::ArtistInfo&, const LocalCollection&);
	};

	using SimilarModel = Util::RoledItemsModel<SimilarArtistInfo>;
	SimilarModel* MakeSimilarModel (QObject *parent);
}
