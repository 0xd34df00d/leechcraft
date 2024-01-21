/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "microcsshandler.h"
#include <util/sll/visitor.h>
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

		void ConvertRule (const StylingContext& ctx, BlockFormat& bfmt, CharFormat&, ImageFormat& ifmt, const Rule& rule)
		{
			if (rule.Property_ == "text-align"_ql)
				bfmt.Align_ = ParseAlign (rule.Value_);
		}

		void ConvertRules (const StylingContext& ctx, BlockFormat& bfmt, CharFormat& cfmt, ImageFormat& ifmt, const QVector<Rule>& rules)
		{
			for (const auto& rule : rules)
				ConvertRule (ctx, bfmt, cfmt, ifmt, rule);
		}

		bool SelectorMatches (const Selector& selector, const StylingContext& ctx)
		{
			// TODO
			if (!selector.Context_.isEmpty ())
				return false;

			const auto& elem = ctx.Elem_;
			return Util::Visit (selector.Head_,
					[] (const AtSelector&) { return false; },
					[&] (const TagSelector& s) { return elem.Tag_ == s.Tag_; },
					[&] (const ClassSelector& s) { return elem.Classes_.contains (s.Class_); },
					[&] (const TagClassSelector& s) { return elem.Tag_ == s.Tag_ && elem.Classes_.contains (s.Class_); },
					[&] (const ComplexSelector& s) { return s (elem); });
		}

		Style Match (const StylingContext& ctx, const Stylesheet& css)
		{
			BlockFormat bfmt;
			CharFormat cfmt;
			ImageFormat ifmt;
			for (const auto& [selector, rules] : css.Selectors_)
				if (SelectorMatches (selector, ctx))
					ConvertRules (ctx, bfmt, cfmt, ifmt, rules);

			return { bfmt, cfmt, ifmt };
		}
	}

	CustomStyler_f MakeStyler (const Stylesheet& css)
	{
		return [css] (const StylingContext& ctx) { return Match (ctx, css); };
	}
}
