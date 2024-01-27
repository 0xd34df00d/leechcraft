/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "types.h"

class QDomElement;

namespace LC::Monocle
{
	struct StylingContext;

	class ImgHandler
	{
		QTextCursor& Cursor_;
		const CustomStyler_f& Styler_;
		const LazyImages_t& Images_;
	public:
		explicit ImgHandler (QTextCursor&, const CustomStyler_f&, const LazyImages_t&);

		void HandleImg (const QDomElement&, const StylingContext&);
	};
}
