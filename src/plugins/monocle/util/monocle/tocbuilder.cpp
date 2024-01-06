/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tocbuilder.h"
#include <QDomElement>
#include <QTextCursor>
#include <util/sll/qtutil.h>
#include <interfaces/monocle/idocument.h>
#include "pagelink.h"

namespace LC::Monocle
{
	TocBuilder::TocBuilder (const QTextCursor& cursor, IDocument& monocleDoc)
	: Cursor_ { cursor }
	, MonocleDoc_ { monocleDoc }
	{
		CurrentSectionPath_.push (&Root_);
	}

	TOCEntryLevel_t TocBuilder::GetTOC () const
	{
		return Root_.ChildLevel_;
	}

	Util::DefaultScopeGuard TocBuilder::HandleElem (const QDomElement& elem)
	{
		const auto& sectionTitle = elem.attribute ("section-title"_qs);
		if (sectionTitle.isEmpty ())
			return {};

		const auto curPosition = Cursor_.position ();
		auto link = std::make_shared<PageLink> (PageLink::LinkInfo
				{
					.MonocleDoc_ = MonocleDoc_,
					.TextDoc_ = *Cursor_.document (),
					.Target_ = Span { curPosition, curPosition },
				});

		auto& curLevel = CurrentSectionPath_.top ()->ChildLevel_;
		curLevel.append ({ .Link_ = std::move (link), .Name_ = sectionTitle, .ChildLevel_ = {} });
		CurrentSectionPath_.push (&curLevel.back ());

		return Util::MakeScopeGuard ([this] { CurrentSectionPath_.pop (); });
	}
}
