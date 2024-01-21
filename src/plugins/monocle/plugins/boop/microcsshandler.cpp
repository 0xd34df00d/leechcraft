/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "microcsshandler.h"
#include <QTextCursor>
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

		std::optional<qreal> ParseDim (const StylingContext& ctx, QStringView str)
		{
			if (str.endsWith ("px"_ql))
			{
				bool ok = false;
				auto val = str.chopped (2).toDouble (&ok);
				if (!ok)
					return {};
				return val;
			}
			if (str.endsWith ("em"_ql))
			{
				bool ok = false;
				auto val = str.chopped (2).toDouble (&ok);
				if (!ok)
					return {};

				auto font = ctx.Cursor_.charFormat ().font ();
				if (font.pointSize () > 0)
					return font.pointSize () * val;
				if (font.pixelSize () > 0)
					return font.pixelSize () * val;
				return {};
			}

			return {};
		}

		void ConvertRule (const StylingContext& ctx, BlockFormat& bfmt, CharFormat&, ImageFormat& ifmt, const Rule& rule)
		{
			if (rule.Property_ == "text-align"_ql)
				bfmt.Align_ = ParseAlign (rule.Value_);
			if (rule.Property_ == "height"_ql)
				ifmt.Height_ = ParseDim (ctx, rule.Value_);
			if (rule.Property_ == "width"_ql)
				ifmt.Width_ = ParseDim (ctx, rule.Value_);
		}

		void ConvertRules (const StylingContext& ctx, BlockFormat& bfmt, CharFormat& cfmt, ImageFormat& ifmt, const QVector<Rule>& rules)
		{
			for (const auto& rule : rules)
				ConvertRule (ctx, bfmt, cfmt, ifmt, rule);
		}

		bool SelectorMatches (const Selector& selector, const StylingContext& ctx)
		{
			const auto& elem = ctx.Elem_;
			const auto thisMatches = Util::Visit (selector.Head_,
					[] (const AtSelector&) { return false; },
					[&] (const TagSelector& s) { return elem.Tag_ == s.Tag_; },
					[&] (const ClassSelector& s) { return elem.Classes_.contains (s.Class_); },
					[&] (const TagClassSelector& s) { return elem.Tag_ == s.Tag_ && elem.Classes_.contains (s.Class_); },
					[&] (const ComplexSelector& s) { return s (elem); });
			if (!thisMatches)
				return false;

			if (selector.Context_.isEmpty ())
				return true;

			if (ctx.Parents_.isEmpty ())
				return false;

			auto subcontext = selector.Context_;
			auto subhead = subcontext.takeLast ();

			auto subparents = ctx.Parents_;
			auto subelem = subparents.takeLast ();

			return SelectorMatches ({ subhead, subcontext }, { subelem, subparents, ctx.Cursor_ });
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
