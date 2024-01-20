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
	namespace
	{
		std::optional<QRectF> GetTargetArea (const Poppler::LinkDestination& dest)
		{
			if (dest.isChangeLeft () && dest.isChangeTop ())
				return QRectF { { dest.left (), dest.top () }, QSizeF {} };
			return {};
		}
	}

	NavigationAction MakeNavigationAction (const Poppler::LinkDestination& dest)
	{
		return
		{
			.PageNumber_ = dest.pageNumber () - 1,
			.TargetArea_ = GetTargetArea (dest),
			.Zoom_ = dest.isChangeZoom () ? dest.zoom () : std::optional<double> {},
		};
	}

	LinkAction MakeLinkAction (Document& doc, const Poppler::Link& link)
	{
		switch (link.linkType ())
		{
		case Poppler::Link::Goto:
		{
			const auto& gotoLink = dynamic_cast<const Poppler::LinkGoto&> (link);
			const auto& dest = gotoLink.destination ();
			if (!gotoLink.isExternal ())
				return MakeNavigationAction (dest);
			return ExternalNavigationAction
			{
				.TargetDocument_ = gotoLink.fileName (),
				.DocumentNavigation_ = MakeNavigationAction (dest)
			};
		}
		case Poppler::Link::Action:
		{
			const auto& actionLink = dynamic_cast<const Poppler::LinkAction&> (link);
			switch (actionLink.actionType ())
			{
			case Poppler::LinkAction::Print:
				doc.RequestPrinting ();
				return CustomAction { [&doc] { doc.RequestPrinting (); } };
			default:
				qWarning () << "unknown link action" << actionLink.actionType ();
				return NoAction {};
			}
		}
		default:
			qWarning () << "unsupported link type:" << link.linkType ();
			return NoAction {};
		}
	}

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

	LinkAction Link::GetLinkAction () const
	{
		return MakeLinkAction (*Doc_, *Link_);
	}
}
}
}
