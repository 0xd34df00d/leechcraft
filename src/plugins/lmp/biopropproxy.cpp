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

#include "biopropproxy.h"
#include <algorithm>

namespace LeechCraft
{
namespace LMP
{
	BioPropProxy::BioPropProxy (QObject *parent)
	: QObject (parent)
	{
	}

	void BioPropProxy::SetBio (const Media::ArtistBio& bio)
	{
		Bio_ = bio;

		QStringList tags;
		std::transform (Bio_.BasicInfo_.Tags_.begin (), Bio_.BasicInfo_.Tags_.end (),
				std::back_inserter (tags),
				[] (decltype (Bio_.BasicInfo_.Tags_.front ()) item) { return item.Name_; });
		CachedTags_ = tags.join ("<br />");

		CachedInfo_ = Bio_.BasicInfo_.FullDesc_.isEmpty () ?
				Bio_.BasicInfo_.ShortDesc_ :
				Bio_.BasicInfo_.FullDesc_;
		CachedInfo_.replace ("\n", "<br />");

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
}
}
