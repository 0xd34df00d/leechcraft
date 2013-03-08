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

#include "links.h"
#include <QtDebug>
#include "document.h"

namespace LeechCraft
{
namespace Monocle
{
namespace PDF
{
	Link::Link (Document *doc, Poppler::Link *link)
	: Doc_ (doc)
	, Link_ (link)
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

	void Link::Execute ()
	{
		switch (GetLinkType ())
		{
		case LinkType::PageLink:
			ExecutePageLink ();
			return;
		default:
			return;
		}
	}

	void Link::ExecutePageLink ()
	{
		auto link = std::dynamic_pointer_cast<Poppler::LinkGoto> (Link_);
		const QString& filename = link->isExternal () ?
				link->fileName () :
				QString ();
		const auto& dest = link->destination ();
		Doc_->RequestNavigation (filename, dest.pageNumber () - 1, dest.left (), dest.top ());
	}

	TOCLink::TOCLink (Document *doc, Poppler::LinkDestination *dest)
	: Doc_ (doc)
	, Dest_ (dest)
	{
	}

	LinkType TOCLink::GetLinkType () const
	{
		return LinkType::PageLink;
	}

	QRectF TOCLink::GetArea () const
	{
		return QRectF ();
	}

	void TOCLink::Execute ()
	{
		Doc_->RequestNavigation (QString (), Dest_->pageNumber () - 1, Dest_->left (), Dest_->top ());
	}
}
}
}
