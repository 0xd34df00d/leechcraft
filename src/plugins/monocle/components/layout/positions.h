/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/monocle/coordsbase.h>

class QGraphicsView;

namespace LC::Monocle
{
	class PageGraphicsItem;

	struct PageRelativePos;
	struct PageAbsolutePos;
	struct SceneAbsolutePos;

	struct PageRelativePos : PageRelativePosBase
	{
		using PageRelativePosBase::PageRelativePosBase;

		PageRelativePos (PageRelativePosBase);

		PageAbsolutePos ToPageAbsolute (const PageGraphicsItem&) const;
		SceneAbsolutePos ToSceneAbsolute (const PageGraphicsItem&) const;
	};

	struct PageAbsolutePos : PageAbsolutePosBase
	{
		using PageAbsolutePosBase::PageAbsolutePosBase;

		PageAbsolutePos (PageAbsolutePosBase);

		PageRelativePos ToPageRelative (const PageGraphicsItem&) const;
		SceneAbsolutePos ToSceneAbsolute (const PageGraphicsItem&) const;
	};

	struct SceneAbsolutePos : Pos<Relativity::SceneAbsolute>
	{
		using Pos::Pos;

		PageAbsolutePos ToPageAbsolute (const PageGraphicsItem&) const;
		PageRelativePos ToPageRelative (const PageGraphicsItem&) const;
	};

	struct ViewAbsolutePos : Pos<Relativity::ViewAbsolute>
	{
		using Pos::Pos;

		SceneAbsolutePos ToSceneAbsolute (const QGraphicsView&) const;
	};

	template<typename T>
	struct PageWithPos
	{
		int Page_;
		T Pos_;
	};

	using PageWithRelativePos = PageWithPos<PageRelativePos>;
	using PageWithAbsolutePos = PageWithPos<PageAbsolutePos>;


	struct PageRelativeRect;
	struct PageAbsoluteRect;
	struct SceneAbsoluteRect;

	struct PageRelativeRect : PageRelativeRectBase
	{
		using PageRelativeRectBase::PageRelativeRectBase;

		PageRelativeRect (const PageRelativeRectBase&);

		PageAbsoluteRect ToPageAbsolute (const PageGraphicsItem&) const;
		SceneAbsoluteRect ToSceneAbsolute (const PageGraphicsItem&) const;
	};

	struct PageAbsoluteRect : PageAbsoluteRectBase
	{
		using PageAbsoluteRectBase::PageAbsoluteRectBase;

		PageAbsoluteRect (const PageAbsoluteRectBase&);

		PageRelativeRect ToPageRelative (const PageGraphicsItem&) const;
		SceneAbsoluteRect ToSceneAbsolute (const PageGraphicsItem&) const;
	};

	struct SceneAbsoluteRect : Rect<SceneAbsoluteRect, Relativity::SceneAbsolute>
	{
		using Rect::Rect;

		PageAbsoluteRect ToPageAbsolute (const PageGraphicsItem&) const;
		PageRelativeRect ToPageRelative (const PageGraphicsItem&) const;
	};

	struct ViewAbsoluteRect : Rect<ViewAbsoluteRect, Relativity::ViewAbsolute>
	{
		using Rect::Rect;

		explicit ViewAbsoluteRect (const QGraphicsView&);

		SceneAbsoluteRect ToSceneAbsolute (const QGraphicsView&) const;
	};
}
