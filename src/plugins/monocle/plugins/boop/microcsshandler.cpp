/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "microcsshandler.h"
#include <util/sll/qtutil.h>
#include <util/monocle/types.h>
#include "microcssparser.h"

namespace LC::Monocle::Boop::MicroCSS
{
	namespace
	{
		std::optional<Qt::AlignmentFlag> ParseAlign (const QString& str)
		{
			if (str == "right"_ql)
				return Qt::AlignRight;
			if (str == "center"_ql)
				return Qt::AlignHCenter;
			if (str == "left"_ql)
				return Qt::AlignLeft;
			if (str == "justify"_ql)
				return Qt::AlignJustify;

			return {};
		}

		void ConvertRule (BlockFormat& bfmt, CharFormat&, const Rule& rule)
		{
			if (rule.Property_ == "text-align"_ql)
			{
				if (const auto align = ParseAlign (rule.Value_))
					bfmt.Align_ = align;
			}
		}

		void ConvertRules (BlockFormat& bfmt, CharFormat& cfmt, const QVector<Rule>& rules)
		{
			for (const auto& rule : rules)
				ConvertRule (bfmt, cfmt, rule);
		}

		Style Match (const StylingContext& ctx, const Stylesheet& css)
		{
			BlockFormat bfmt;
			CharFormat cfmt;
			ImageFormat ifmt;
			ConvertRules (bfmt, cfmt, css.Selectors_ [TagSelector { ctx.Tag_.toString () }]);
			for (const auto& klass : ctx.Classes_)
				ConvertRules (bfmt, cfmt, css.Selectors_ [ClassSelector { klass.toString () }]);

			return { bfmt, cfmt, ifmt };
		}
	}

	CustomStyler_f MakeStyler (const Stylesheet& css)
	{
		return [css] (const StylingContext& ctx) { return Match (ctx, css); };
	}
}
