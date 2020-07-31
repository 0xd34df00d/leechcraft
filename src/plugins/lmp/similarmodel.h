/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStandardItemModel>
#include <util/models/rolenamesmixin.h>

namespace Media
{
	struct ArtistInfo;
}

namespace LC
{
namespace LMP
{
	class SimilarModel : public Util::RoleNamesMixin<QStandardItemModel>
	{
		Q_OBJECT
	public:
		enum Role
		{
			ArtistName = Qt::UserRole + 1,
			Similarity,
			ArtistImageURL,
			ArtistBigImageURL,
			ArtistPageURL,
			ArtistTags,
			ShortDesc,
			FullDesc,
			IsInCollection
		};

		SimilarModel (QObject* = 0);

		static QStandardItem* ConstructItem (const Media::ArtistInfo&);
	};
}
}
