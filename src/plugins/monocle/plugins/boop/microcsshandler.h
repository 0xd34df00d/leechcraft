/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>

class QTextCharFormat;
class QTextBlockFormat;

namespace LC::Monocle
{
	struct CharFormat;
	struct BlockFormat;

	struct StylingContext;
	struct Style;
}

namespace LC::Monocle::Boop::MicroCSS
{
	struct Stylesheet;

	using CustomStyler_f = std::function<Style (const StylingContext&)>;

	CustomStyler_f MakeStyler (const Stylesheet&);
}
