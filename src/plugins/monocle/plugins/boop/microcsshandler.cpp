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

		bool SelectorMatches (const ManyClassesSelector& s, const StylingContextElement& elem)
		{
			if (!s.Tag_.isEmpty () && s.Tag_ != elem.Tag_)
				return false;

			return std::all_of (s.Classes_.begin (), s.Classes_.end (),
					[&] (auto klass) { return elem.Classes_.contains (klass); });
		}

		template<typename Item>
		struct TailView
		{
			const Item& Elem_;

			QVector<Item>::const_iterator Begin_;
			QVector<Item>::const_iterator End_;

			TailView (const Item& elem, QVector<Item>::const_iterator begin, QVector<Item>::const_iterator end)
			: Elem_ { elem }
			, Begin_ { begin }
			, End_ { end }
			{
			}

			TailView (const Item& elem, const QVector<Item>& rest)
			: Elem_ { elem }
			, Begin_ { rest.begin () }
			, End_ { rest.end () }
			{
			}

			bool IsTailEmpty () const
			{
				return Begin_ == End_;
			}

			TailView Pop () const
			{
				auto newEnd = End_ - 1;
				return { *newEnd, Begin_, newEnd };
			}
		};

		bool SelectorMatches (const TailView<SingleSelector>& selector,
				const TailView<StylingContextElement>& ctx)
		{
			const auto& elem = ctx.Elem_;
			const auto thisMatches = Util::Visit (selector.Elem_,
					[] (const AtSelector&) { return false; },
					[&] (const TagSelector& s) { return elem.Tag_ == s.Tag_; },
					[&] (const ClassSelector& s) { return elem.Classes_.contains (s.Class_); },
					[&] (const TagClassSelector& s) { return elem.Tag_ == s.Tag_ && elem.Classes_.contains (s.Class_); },
					[&] (const ManyClassesSelector& s) { return SelectorMatches (s, elem); });
			if (!thisMatches)
				return false;

			if (selector.IsTailEmpty ())
				return true;

			if (ctx.IsTailEmpty ())
				return false;

			return SelectorMatches (selector.Pop (), ctx.Pop ());
		}

		Style Match (const StylingContext& ctx, const Stylesheet& css)
		{
			BlockFormat bfmt;
			CharFormat cfmt;
			ImageFormat ifmt;
			const auto& tag = Util::UnsafeFromView (ctx.Elem_.Tag_);
			ConvertRules (ctx, bfmt, cfmt, ifmt, css.ByTag_ [TagSelector { tag }]);
			for (const auto& klassView : ctx.Elem_.Classes_)
			{
				const auto& klass = Util::UnsafeFromView (klassView);
				ConvertRules (ctx, bfmt, cfmt, ifmt, css.ByClass_ [ClassSelector { klass }]);
				ConvertRules (ctx, bfmt, cfmt, ifmt, css.ByTagAndClass_ [TagClassSelector { tag, klass }]);
				for (const auto& [selector, rules] : css.ManyClassesByTag_ [tag])
					if (SelectorMatches (selector, ctx.Elem_))
						ConvertRules (ctx, bfmt, cfmt, ifmt, rules);
			}
			for (const auto& [selector, rules] : css.ComplexByTag_ [tag])
				if (SelectorMatches ({ selector.Head_, selector.Context_ }, { ctx.Elem_, ctx.Parents_ }))
					ConvertRules (ctx, bfmt, cfmt, ifmt, rules);
			for (const auto& [selector, rules] : css.Others_)
				if (SelectorMatches ({ selector.Head_, selector.Context_ }, { ctx.Elem_, ctx.Parents_ }))
					ConvertRules (ctx, bfmt, cfmt, ifmt, rules);

			return { bfmt, cfmt, ifmt };
		}
	}

	CustomStyler_f MakeStyler (const Stylesheet& css)
	{
		return [css] (const StylingContext& ctx) { return Match (ctx, css); };
	}
}
