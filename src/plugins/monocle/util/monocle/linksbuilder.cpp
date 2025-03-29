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
#include <QUrl>
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
		Util::DefaultScopeGuard targetGuard;
		Util::DefaultScopeGuard linkGuard;
		if (const auto& id = elem.attribute ("id"_qs); !id.isEmpty ())
			targetGuard = HandleTarget (id);
		if (elem.tagName () == "a"_ql)
			linkGuard = HandleLink (elem);

		return Util::DefaultScopeGuard { std::move (targetGuard), std::move (linkGuard) };
	}

	QVector<InternalLink> LinksBuilder::GetLinks () const
	{
		QVector<InternalLink> links;
		links.reserve (Sources_.size ());
		for (const auto& [id, anchorSources] : Util::Stlize (Sources_))
		{
			const auto targetPos = Targets_.find (id);
			if (targetPos == Targets_.end ())
			{
				qWarning () << "unknown target" << id;
				continue;
			}

			for (const auto& source : anchorSources)
				links.append ({ .LinkTitle_ = source.Title_, .Link_ = source.Span_, .Target_ = *targetPos });
		}
		return links;
	}

	Util::DefaultScopeGuard LinksBuilder::HandleTarget (const QString& anchorId)
	{
		const auto start = Cursor_.position ();
		Targets_ [anchorId] = Span { start, -1 };
		return Util::MakeScopeGuard ([this, anchorId] { Targets_ [anchorId].End_ = Cursor_.position (); });
	}

	Util::DefaultScopeGuard LinksBuilder::HandleLink (const QDomElement& elem)
	{
		const auto& href = QUrl::fromPercentEncoding (elem.attribute ("href"_qs).toUtf8 ());
		if (!href.startsWith ('#'))
			return {};

		const auto& title = elem.attribute ("title"_qs);
		const auto anchor = href.mid (1);
		const auto start = Cursor_.position ();

		auto& anchorSources = Sources_ [anchor];
		const auto srcIdx = anchorSources.size ();
		anchorSources.push_back ({ title, { start, -1 } });
		return Util::MakeScopeGuard ([this, anchor, srcIdx]
				{
					Sources_ [anchor] [srcIdx].Span_.End_ = Cursor_.position ();
				});
	}
}
