/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tocbuilder.h"
#include <QDomElement>
#include <util/sll/qtutil.h>
#include <util/monocle/types.h>

namespace LC::Monocle::FXB
{
	TocBuilder::TocBuilder ()
	: CurrentEntryPath_ { { &Root_ } }
	{
	}

	TOCEntryID TocBuilder::GetToc () const
	{
		return Root_;
	}

	namespace
	{
		QString ExtractTitle (const QDomElement& section)
		{
			const auto& title = section.firstChildElement ("title"_qs);
			const auto& p = title.firstChildElement ("p"_qs);
			if (!p.isNull ())
				return p.text ();

			constexpr auto sectionNameSize = 50;
			auto result = title.text ();
			if (result.size () > sectionNameSize)
			{
				result.truncate (sectionNameSize);
				result += u"…"_qs;
			}
			return result;
		}
	}

	Util::DefaultScopeGuard TocBuilder::HandleElem (QDomElement elem)
	{
		const auto& tagName = elem.tagName ();
		if (tagName != "section"_ql && tagName != "subsection"_ql)
			return {};

		++IdCounter_;

		elem.setAttribute (TocSectionIdAttr, IdCounter_);

		auto& curLevel = CurrentEntryPath_.top ()->ChildLevel_;
		curLevel.append ({ .Navigation_ = QByteArray::number (IdCounter_), .Name_ = ExtractTitle (elem) });
		CurrentEntryPath_.push (&curLevel.back ());

		return Util::MakeScopeGuard ([this] { CurrentEntryPath_.pop (); });
	}

}
