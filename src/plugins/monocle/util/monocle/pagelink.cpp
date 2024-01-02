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
	PageLink::PageLink (IDocument& monocleDoc, const QTextDocument& textDoc, Span targetSpan, std::optional<Span> sourceSpan)
	: MonocleDoc_ { monocleDoc }
	, TextDoc_ { textDoc }
	, TargetSpan_ { targetSpan }
	, SourceSpan_ { sourceSpan }
	{
	}

	LinkType PageLink::GetLinkType () const
	{
		return LinkType::PageLink;
	}

	QRectF PageLink::GetArea () const
	{
		if (!SourceSpan_)
			return {};

		return ComputeArea (*SourceSpan_, CachedSource_).Area_;
	}

	void PageLink::Execute ()
	{
		MonocleDoc_.navigateRequested ({}, { .Page_ = GetPageNumber (), .PagePosition_ = GetTargetArea () });
	}

	QString PageLink::GetDocumentFilename () const
	{
		return {};
	}

	namespace
	{
		QRectF GetPosRect (int docPos, const QTextDocument& doc)
		{
			const auto docLayout = doc.documentLayout ();

			const auto block = doc.findBlock (docPos);
			const auto& blockRect = docLayout->blockBoundingRect (block);

			const auto blockLayout = block.layout ();
			const auto inBlockPos = docPos - block.position ();
			const auto line = blockLayout->lineForTextPosition (inBlockPos);
			if (!line.isValid ())
				return QRectF { blockRect.topLeft (), QSizeF {} };

			auto lineShift = line.position ();
			const auto inLinePos = inBlockPos - line.textStart ();
			lineShift.rx () += line.cursorToX (inLinePos);

			return { blockRect.topLeft () + lineShift, QSizeF {} };
		}

		QRectF GetSpanRect (Span span, const QTextDocument& doc)
		{
			return GetPosRect (span.Start_, doc) | GetPosRect (span.End_, doc);
		}
	}

	int PageLink::GetPageNumber () const
	{
		return ComputeArea (TargetSpan_, CachedTarget_).Page_;
	}

	std::optional<QRectF> PageLink::GetTargetArea () const
	{
		return ComputeArea (TargetSpan_, CachedTarget_).Area_;
	}

	std::optional<double> PageLink::GetNewZoom () const
	{
		return {};
	}

	const AreaInfo& PageLink::ComputeArea (Span span, std::optional<AreaInfo>& areaInfo) const
	{
		if (areaInfo)
			return *areaInfo;

		const auto spanRect = GetSpanRect (span, TextDoc_);
		const auto shiftY = spanRect.toRect ().top ();
		const auto pageSize = TextDoc_.pageSize ();

		const auto quotrem = std::div (shiftY, static_cast<int> (pageSize.height ()));

		auto pageArea = spanRect;
		pageArea.moveTop (quotrem.rem);

		pageArea.moveTop (pageArea.top () / pageSize.height ());
		pageArea.setHeight (pageArea.height () / pageSize.height ());
		pageArea.moveLeft (pageArea.height () / pageSize.width ());
		pageArea.setWidth (pageArea.height () / pageSize.width ());

		areaInfo = AreaInfo { .Page_ = quotrem.quot, .Area_ = pageArea };
		return *areaInfo;
	}
}
