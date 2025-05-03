/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "biopropproxy.h"
#include <algorithm>
#include <QtDebug>
#include <util/models/itemsmodel.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>

namespace LC::LMP
{
	BioPropProxy::BioPropProxy (QObject *parent)
	: QObject { parent }
	, ArtistImages_ { new ArtistImagesModel_t {
			this,
			Util::RoledMemberField_v<"thumbURL", &Media::ArtistImage::Thumb_>,
			Util::RoledMemberField_v<"fullURL", &Media::ArtistImage::Full_>,
			Util::RoledMemberField_v<"title", &Media::ArtistImage::Title_>,
			Util::RoledMemberField_v<"author", &Media::ArtistImage::Author_>,
			Util::RoledMemberField_v<"date", &Media::ArtistImage::Date_>,
	} }
	{
	}

	void BioPropProxy::SetBio (const Media::ArtistBio& bio)
	{
		if (Bio_ == bio)
			return;

		Bio_ = bio;

		CachedTags_ = Util::Map (Bio_.BasicInfo_.Tags_, &Media::TagInfo::Name_).join (QStringLiteral ("; "));

		CachedInfo_ = Bio_.BasicInfo_.FullDesc_.isEmpty () ?
				Bio_.BasicInfo_.ShortDesc_ :
				Bio_.BasicInfo_.FullDesc_;
		CachedInfo_.replace ("\n"_ql, "<br />"_ql);

		ArtistImages_->SetItems (bio.OtherImages_.toVector ());

		emit artistNameChanged (GetArtistName ());
		emit artistImageURLChanged (GetArtistImageURL ());
		emit artistBigImageURLChanged (GetArtistBigImageURL ());
		emit artistTagsChanged (GetArtistTags ());
		emit artistInfoChanged (GetArtistInfo ());
		emit artistPageURLChanged (GetArtistPageURL ());
	}

	QString BioPropProxy::GetArtistName () const
	{
		return Bio_.BasicInfo_.Name_;
	}

	QUrl BioPropProxy::GetArtistImageURL () const
	{
		return Bio_.BasicInfo_.Image_;
	}

	QUrl BioPropProxy::GetArtistBigImageURL () const
	{
		return Bio_.BasicInfo_.LargeImage_;
	}

	QString BioPropProxy::GetArtistTags () const
	{
		return CachedTags_;
	}

	QString BioPropProxy::GetArtistInfo () const
	{
		return CachedInfo_;
	}

	QUrl BioPropProxy::GetArtistPageURL () const
	{
		return Bio_.BasicInfo_.Page_;
	}

	QObject* BioPropProxy::GetArtistImagesModel () const
	{
		return ArtistImages_;
	}
}
