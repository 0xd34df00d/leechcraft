/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pagelink.h"
#include <interfaces/monocle/idocument.h>

namespace LC::Monocle
{
	PageLink::PageLink (const QRectF& area, int targetPage, const QRectF& targetArea, IDocument *doc)
	: LinkArea_ { area }
	, TargetPage_ { targetPage }
	, TargetArea_ { targetArea }
	, Doc_ { doc }
	{
	}

	LinkType PageLink::GetLinkType () const
	{
		return LinkType::PageLink;
	}

	QRectF PageLink::GetArea () const
	{
		return LinkArea_;
	}

	void PageLink::Execute ()
	{
		Doc_.navigateRequested ({}, { GetPageNumber (), GetTargetArea () });
	}

	QString PageLink::GetDocumentFilename () const
	{
		return {};
	}

	int PageLink::GetPageNumber () const
	{
		return TargetPage_;
	}

	std::optional<QRectF> PageLink::GetTargetArea () const
	{
		return TargetArea_;
	}

	std::optional<double> PageLink::GetNewZoom () const
	{
		return {};
	}
}
