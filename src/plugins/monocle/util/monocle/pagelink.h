/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/monocle/ilink.h>

namespace LC::Monocle
{
	class IDocument;

	class PageLink : public ILink
				   , public IPageLink
	{
		const QRectF LinkArea_;

		const int TargetPage_;
		const QRectF TargetArea_;

		IDocument * const Doc_;
	public:
		PageLink (const QRectF& area, int targetPage, const QRectF& targetArea, IDocument *doc);

		LinkType GetLinkType () const override;
		QRectF GetArea () const override;
		void Execute () override;
		QString GetDocumentFilename () const override;
		int GetPageNumber () const override;
		double NewX () const override;
		double NewY () const override;
		double NewZoom () const override;
	};
}
