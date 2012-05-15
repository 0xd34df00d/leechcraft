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

#include "links.h"

namespace LeechCraft
{
namespace Monocle
{
namespace PDF
{
	Link::Link (Poppler::Link *link)
	: Link_ (link)
	{
	}

	LinkType Link::GetLinkType () const
	{
		switch (Link_->linkType ())
		{
		case Poppler::Link::Goto:
			return LinkType::PageLink;
		case Poppler::Link::Browse:
			return LinkType::URL;
		case Poppler::Link::Action:
			return LinkType::Command;
		default:
			return LinkType::OtherLink;
		}
	}

	QRectF Link::GetArea () const
	{
		return Link_->linkArea ();
	}

	void Link::Execute () const
	{
	}

	PageLink::PageLink (Poppler::LinkGoto *link)
	: Link (link)
	, X_ (0)
	, Y_ (0)
	, Zoom_ (0)
	{
		LinkGoto_ = std::dynamic_pointer_cast<Poppler::LinkGoto> (Link_);

		const auto& dest = LinkGoto_->destination ();
		if (dest.kind () == Poppler::LinkDestination::destXYZ)
		{
			X_ = dest.left ();
			Y_ = dest.top ();
			Zoom_ = dest.zoom ();
		}
	}

	QString PageLink::GetDocumentFilename () const
	{
		return LinkGoto_->isExternal () ?
				LinkGoto_->destination ().destinationName () :
				QString ();
	}

	int PageLink::GetPageNumber () const
	{
		return LinkGoto_->destination ().pageNumber ();
	}

	double PageLink::NewX () const
	{
		return X_;
	}

	double PageLink::NewY () const
	{
		return Y_;
	}

	double PageLink::NewZoom () const
	{
		return Zoom_;
	}
}
}
}