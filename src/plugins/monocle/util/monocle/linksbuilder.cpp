/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "linksbuilder.h"
#include <QDomElement>
#include <QTextCursor>
#include <QtDebug>
#include <util/sll/qtutil.h>

namespace LC::Monocle
{
	LinksBuilder::LinksBuilder (const QTextCursor& cursor)
	: Cursor_ { cursor }
	{
	}

	Util::DefaultScopeGuard LinksBuilder::HandleElem (const QDomElement& elem)
	{
		if (const auto& id = elem.attribute ("id"_qs); !id.isEmpty ())
			return HandleTarget (id);
		if (elem.tagName () == "a"_ql)
			return HandleLink (elem);

		return {};
	}

	QVector<InternalLink> LinksBuilder::GetLinks () const
	{
		QVector<InternalLink> links;
		links.reserve (Sources_.size ());
		for (const auto& [id, sourceSpan] : Util::Stlize (Sources_))
		{
			const auto targetPos = Targets_.find (id);
			if (targetPos == Targets_.end ())
			{
				qWarning () << "unknown target" << id;
				continue;
			}

			links.append ({ .Link_ = sourceSpan, .Target_ = *targetPos });
		}
		return links;
	}

	Util::DefaultScopeGuard LinksBuilder::HandleTarget (const QString& anchorId)
	{
		return Util::MakeScopeGuard ([this, anchorId, start = Cursor_.position ()]
				{
					Targets_ [anchorId] = Span { start, Cursor_.position () };
				});
	}

	Util::DefaultScopeGuard LinksBuilder::HandleLink (const QDomElement& elem)
	{
		const auto& href = elem.attribute ("href"_qs);
		if (!href.startsWith ('#'))
			return {};

		return Util::MakeScopeGuard ([this, anchor = href.mid (1), start = Cursor_.position ()]
				{
					Sources_ [anchor] = Span { start, Cursor_.position () };
				});
	}
}
