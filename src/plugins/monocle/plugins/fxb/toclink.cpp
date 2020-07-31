/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "toclink.h"
#include "document.h"

namespace LC
{
namespace Monocle
{
namespace FXB
{
	TOCLink::TOCLink (Document *doc, int page)
	: Doc_ (doc)
	, Page_ (page)
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
		Doc_->RequestNavigation (Page_);
	}
}
}
}
