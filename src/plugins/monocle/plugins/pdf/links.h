/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/monocle/ilink.h>

namespace Poppler
{
	class Link;
	class LinkDestination;
}

namespace LC::Monocle::PDF
{
	class Document;

	NavigationAction MakeNavigationAction (const Poppler::LinkDestination&);

	LinkAction MakeLinkAction (Document&, const Poppler::Link&);

	class Link : public QObject
			   , public ILink
	{
		Q_INTERFACES (LC::Monocle::ILink)

		LinkType Type_;
		PageRelativeRectBase Area_;
		LinkAction Action_;
	public:
		explicit Link (Document&, const Poppler::Link&);

		LinkType GetLinkType () const override;
		PageRelativeRectBase GetArea () const override;
		LinkAction GetLinkAction () const override;
	};
}
