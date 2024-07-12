/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/monocle/coords.h>

namespace LC::Monocle
{
	class PageGraphicsItem;

	struct PageRelativePos;
	struct PageAbsolutePos;
	struct SceneAbsolutePos;

	struct PageRelativePos : PageRelativePosBase
	{
		using PageRelativePosBase::PageRelativePosBase;

		PageAbsolutePos ToPageAbsolute (const PageGraphicsItem&) const;
		SceneAbsolutePos ToSceneAbsolute (const PageGraphicsItem&) const;
	};

	struct PageAbsolutePos : Pos<PageAbsolutePos>
	{
		using Pos::Pos;

		PageRelativePos ToPageRelative (const PageGraphicsItem&) const;
		SceneAbsolutePos ToSceneAbsolute (const PageGraphicsItem&) const;
	};

	struct SceneAbsolutePos : Pos<SceneAbsolutePos>
	{
		using Pos::Pos;

		PageAbsolutePos ToPageAbsolute (const PageGraphicsItem&) const;
		PageRelativePos ToPageRelative (const PageGraphicsItem&) const;
	};

	template<typename T>
	struct PageWithPos
	{
		int Page_;
		T Pos_;
	};

	using PageWithRelativePos = PageWithPos<PageRelativePos>;
}
