/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "links.h"
#include <QtDebug>
#include "document.h"

namespace LC
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

	Link::Link (Document *doc, Poppler::Link *link, const std::shared_ptr<void>& ptr)
	: Doc_ (doc)
	, Link_ (ptr, link)
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
		case LinkType::Command:
			ExecuteCommandLink ();
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

	void Link::ExecuteCommandLink ()
	{
		auto link = std::dynamic_pointer_cast<Poppler::LinkAction> (Link_);
		switch (link->actionType ())
		{
		case Poppler::LinkAction::Print:
			Doc_->RequestPrinting ();
			break;
		default:
			break;
		}
	}

	TOCLink::TOCLink (Document *doc, std::unique_ptr<Poppler::LinkDestination> dest)
	: Doc_ (doc)
	, Dest_ (std::move (dest))
	{
	}

	LinkType TOCLink::GetLinkType () const
	{
		return LinkType::PageLink;
	}

	QRectF TOCLink::GetArea () const
	{
		return {};
	}

	void TOCLink::Execute ()
	{
		Doc_->RequestNavigation (QString (), Dest_->pageNumber () - 1, Dest_->left (), Dest_->top ());
	}

	QString TOCLink::GetDocumentFilename () const
	{
		return {};
	}

	int TOCLink::GetPageNumber () const
	{
		return Dest_->pageNumber () - 1;
	}

	std::optional<QRectF> TOCLink::GetTargetArea () const
	{
		if (Dest_->isChangeLeft () && Dest_->isChangeTop ())
			return QRectF { QPointF { Dest_->left (), Dest_->top () }, QSizeF {} };
		return {};
	}

	std::optional<double> TOCLink::GetNewZoom () const
	{
		if (Dest_->isChangeZoom ())
			return Dest_->zoom ();
		return {};
	}
}
}
}
