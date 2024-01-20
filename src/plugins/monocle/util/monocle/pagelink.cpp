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

namespace LC::Monocle
{
	PageLink::PageLink (const LinkInfo& info)
	: Info_ { info }
	{
	}

	LinkType PageLink::GetLinkType () const
	{
		return LinkType::PageLink;
	}

	QRectF PageLink::GetArea () const
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
		QRectF GetPosRect (int docPos, const QTextDocument& doc)
		{
			const auto docLayout = doc.documentLayout ();

			const auto block = doc.findBlock (docPos);
			const auto& blockPos = docLayout->blockBoundingRect (block).topLeft ();

			const auto blockLayout = block.layout ();
			const auto inBlockPos = docPos - block.position ();
			const auto line = blockLayout->lineForTextPosition (inBlockPos);
			if (!line.isValid ())
				return QRectF { blockPos, QSizeF {} };

			auto lineShift = line.position ();
			lineShift.rx () += line.cursorToX (inBlockPos);

			return { blockPos + lineShift, QSizeF { 1, line.height () } };
		}

		QRectF GetSpanRect (Span span, const QTextDocument& doc)
		{
			return GetPosRect (span.Start_, doc) | GetPosRect (span.End_, doc);
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

		const auto spanRect = GetSpanRect (span, Info_.TextDoc_);
		const auto shiftY = spanRect.toRect ().top ();
		const auto pageSize = Info_.TextDoc_.pageSize ();

		const auto quotrem = std::div (shiftY, static_cast<int> (pageSize.height ()));

		auto pageArea = spanRect;
		pageArea.moveTop (quotrem.rem);

		pageArea.moveTop (pageArea.top () / pageSize.height ());
		pageArea.setHeight (pageArea.height () / pageSize.height ());
		pageArea.moveLeft (pageArea.left () / pageSize.width ());
		pageArea.setWidth (pageArea.width () / pageSize.width ());

		areaInfo = AreaInfo { .Page_ = quotrem.quot, .Area_ = pageArea };
		return *areaInfo;
	}

	NavigationAction PageLink::ComputeNavAction () const
	{
		const auto& computed = ComputeArea (Info_.Target_, CachedTarget_);
		return { .PageNumber_ = computed.Page_, .TargetArea_ = computed.Area_ };
	}
}
