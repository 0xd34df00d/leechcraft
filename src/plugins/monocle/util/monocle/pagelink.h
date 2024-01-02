/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/monocle/ilink.h>
#include "types.h"

class QTextDocument;

namespace LC::Monocle
{
	class IDocument;

	struct AreaInfo
	{
		int Page_;
		QRectF Area_;
	};

	class PageLink : public ILink
				   , public IPageLink
	{
		IDocument& MonocleDoc_;

		const QTextDocument& TextDoc_;

		const Span TargetSpan_;
		const std::optional<Span> SourceSpan_;

		mutable std::optional<AreaInfo> CachedTarget_;
		mutable std::optional<AreaInfo> CachedSource_;
	public:
		PageLink (IDocument& monocleDoc, const QTextDocument& textDoc, Span targetSpan, std::optional<Span> sourceSpan = {});

		LinkType GetLinkType () const override;
		QRectF GetArea () const override;
		void Execute () override;

		QString GetDocumentFilename () const override;
		int GetPageNumber () const override;
		std::optional<QRectF> GetTargetArea () const override;
		std::optional<double> GetNewZoom () const override;
	private:
		const AreaInfo& ComputeArea (Span, std::optional<AreaInfo>&) const;
	};
}
