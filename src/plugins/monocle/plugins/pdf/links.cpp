/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "links.h"
#include <QtDebug>
#include "qt5compat.h"
#include <poppler-qt5.h>
#include "document.h"

namespace LC::Monocle::PDF
{
	namespace
	{
		std::optional<PageRelativeRectBase> GetTargetArea (const Poppler::LinkDestination& dest)
		{
			if (dest.isChangeLeft () && dest.isChangeTop ())
				return PageRelativeRectBase { { { dest.left (), dest.top () }, QSizeF { 0, 0 } } };
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
				return CustomAction { [&doc] { doc.RequestPrinting (); } };
			default:
				qWarning () << "unknown link action" << actionLink.actionType ();
				return NoAction {};
			}
		}
		case Poppler::Link::Browse:
		{
			const auto& browseLink = dynamic_cast<const Poppler::LinkBrowse&> (link);
			return UrlAction { browseLink.url () };
		}
		default:
			qWarning () << "unsupported link type:" << link.linkType ();
			return NoAction {};
		}
	}

	namespace
	{
		auto FromPoppler (Poppler::Link::LinkType type)
		{
			switch (type)
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
	}

	Link::Link (Document& doc, const Poppler::Link& link)
	: Type_ { FromPoppler (link.linkType ()) }
	, Area_ { link.linkArea () }
	, Action_ { MakeLinkAction (doc, link) }
	{
	}

	LinkType Link::GetLinkType () const
	{
		return Type_;
	}

	PageRelativeRectBase Link::GetArea () const
	{
		return Area_;
	}

	LinkAction Link::GetLinkAction () const
	{
		return Action_;
	}
}
