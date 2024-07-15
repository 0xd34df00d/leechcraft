/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pagelink.h"
#include <QAbstractTextDocumentLayout>
#include <QTextBlock>
#include <QTextDocument>
#include <QtDebug>
#include <interfaces/monocle/idocument.h>
#include "components/layout/positions.h"

namespace LC::Monocle
{
	PageLink::PageLink (LinkInfo info)
	: Info_ { std::move (info) }
	{
	}

	LinkType PageLink::GetLinkType () const
	{
		return LinkType::PageLink;
	}

	PageRelativeRectBase PageLink::GetArea () const
	{
		if (!Info_.Source_)
			return {};

		return ComputeArea (*Info_.Source_, CachedSource_).Area_;
	}

	LinkAction PageLink::GetLinkAction () const
	{
		return ComputeNavAction ();
	}

	QString PageLink::GetToolTip () const
	{
		return Info_.ToolTip_;
	}

	namespace
	{
		PageAbsoluteRect::Type GetPosRect (int docPos, const QTextDocument& doc)
		{
			const auto docLayout = doc.documentLayout ();

			const auto block = doc.findBlock (docPos);
			const auto& blockPos = docLayout->blockBoundingRect (block).topLeft ();

			const auto blockLayout = block.layout ();
			const auto inBlockPos = docPos - block.position ();
			const auto line = blockLayout->lineForTextPosition (inBlockPos);
			if (!line.isValid ())
				return { blockPos, QSizeF {} };

			auto lineShift = line.position ();
			lineShift.rx () += line.cursorToX (inBlockPos);
			if (!line.lineNumber ())
				lineShift.rx () -= block.blockFormat ().textIndent ();

			return { blockPos + lineShift, QSizeF { 1, line.height () } };
		}

		PageAbsoluteRect GetSpanRect (Span span, const QTextDocument& doc)
		{
			return PageAbsoluteRect { GetPosRect (span.Start_, doc) | GetPosRect (span.End_, doc) };
		}

		PageRelativeRect ToPageRelative (const PageAbsoluteRect& r, QSizeF pageSize)
		{
			auto rect = r.ToRectF ();
			rect.moveTop (rect.top () / pageSize.height ());
			rect.setHeight (rect.height () / pageSize.height ());
			rect.moveLeft (rect.left () / pageSize.width ());
			rect.setWidth (rect.width () / pageSize.width ());
			return PageRelativeRect { rect };
		}
	}

	NavigationAction PageLink::GetNavigationAction (const LinkInfo& info)
	{
		return PageLink { info }.ComputeNavAction ();
	}

	int PageLink::GetSourcePage () const
	{
		if (!Info_.Source_)
		{
			qWarning () << "no source span";
			return -1;
		}

		return ComputeArea (*Info_.Source_, CachedSource_).Page_;
	}

	const AreaInfo& PageLink::ComputeArea (Span span, std::optional<AreaInfo>& areaInfo) const
	{
		if (areaInfo)
			return *areaInfo;

		auto spanRect = GetSpanRect (span, Info_.TextDoc_);
		const auto shiftY = spanRect.ToRectF ().toRect ().top ();
		const auto pageSize = Info_.TextDoc_.pageSize ();

		const auto quotrem = std::div (shiftY, static_cast<int> (pageSize.height ()));
		spanRect.R_.moveTop (quotrem.rem);
		areaInfo = AreaInfo { .Page_ = quotrem.quot, .Area_ = ToPageRelative (spanRect, pageSize) };
		return *areaInfo;
	}

	NavigationAction PageLink::ComputeNavAction () const
	{
		const auto& computed = ComputeArea (Info_.Target_, CachedTarget_);
		return { .PageNumber_ = computed.Page_, .TargetArea_ = computed.Area_ };
	}
}
