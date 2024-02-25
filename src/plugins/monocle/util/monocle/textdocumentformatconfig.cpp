/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "textdocumentformatconfig.h"
#include <QGuiApplication>
#include <QPalette>
#include <QTextDocument>
#include <QTextFrame>
#include <QtDebug>
#include <util/sll/ctstring.h>
#include <util/sll/qtutil.h>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include "types.h"

namespace LC::Monocle
{
	TextDocumentFormatConfig& TextDocumentFormatConfig::Instance ()
	{
		static TextDocumentFormatConfig tdfc;
		return tdfc;
	}

	const TextDocumentPalette& TextDocumentFormatConfig::GetPalette () const
	{
		return Palette_;
	}

	void TextDocumentFormatConfig::FormatDocument (QTextDocument& doc) const
	{
		const QSize pageSize { XSM_->property ("PageWidth").toInt (), XSM_->property ("PageHeight").toInt () };
		doc.setPageSize (pageSize);
		doc.setUndoRedoEnabled (false);
		doc.setDefaultFont (XSM_->property ("DefaultFont").value<QFont> ());
	}

	namespace
	{
		template<Util::CtString Prefix>
		QMargins GetMargins (Util::BaseSettingsManager& xsm)
		{
			constexpr auto side = [] (char side) consteval { return Prefix + side + '\0'; };
			const auto margin = [&xsm] (auto side) { return xsm.property (side.Data ()).toInt (); };
			return
			{
				margin (side ('L')),
				margin (side ('T')),
				margin (side ('R')),
				margin (side ('B')),
			};
		}
	}

	QTextFrameFormat TextDocumentFormatConfig::GetBodyFrameFormat () const
	{
		const auto& margins = GetMargins<"PageMargin"_ct> (*XSM_);
		QTextFrameFormat fmt;
		fmt.setLeftMargin (margins.left ());
		fmt.setRightMargin (margins.right ());
		fmt.setTopMargin (margins.top ());
		fmt.setBottomMargin (margins.bottom ());
		fmt.setWidth (XSM_->property ("PageWidth").toInt ());
		return fmt;
	}

	void TextDocumentFormatConfig::SetXSM (Util::BaseSettingsManager& xsm)
	{
		XSM_ = &xsm;

		xsm.RegisterObject ({ "CustomColors", "ColorBackground", "ColorForeground", "ColorLink" },
				this, [this] { UpdatePalette (); });
		xsm.RegisterObject ({ "ParaMarginL", "ParaMarginR", "ParaMarginT", "ParaMarginB", "ParaIndent" },
				this, [this] { UpdateParaBlockFormat (); });
	}

	void TextDocumentFormatConfig::UpdatePalette ()
	{
		if (XSM_->property ("CustomColors").toBool ())
		{
			Palette_ =
			{
				.Background_ = XSM_->property ("ColorBackground").value<QColor> (),
				.Foreground_ = XSM_->property ("ColorForeground").value<QColor> (),
				.Link_ = XSM_->property ("ColorLink").value<QColor> (),
			};
		}
		else
		{
			const auto& palette = qGuiApp->palette ();
			Palette_ =
			{
				.Background_ = palette.color (QPalette::Base),
				.Foreground_ = palette.color (QPalette::Text),
				.Link_ = palette.color (QPalette::Link),
			};
		}
	}

	void TextDocumentFormatConfig::UpdateParaBlockFormat ()
	{
		const auto setNonDefault = [] (auto& optional, auto value)
		{
			if (value >= 0)
				optional = value;
			else
				optional.reset ();
		};

		const auto& margins = GetMargins<"ParaMargin"_ct> (*XSM_);
		setNonDefault (FormatP_.MarginLeft_, margins.left ());
		setNonDefault (FormatP_.MarginRight_, margins.right ());
		setNonDefault (FormatP_.MarginTop_, margins.top ());
		setNonDefault (FormatP_.MarginBottom_, margins.bottom ());

		setNonDefault (FormatP_.Indent_, XSM_->property ("ParaIndent").toInt ());
	}

	namespace
	{
		std::optional<int> GetHeadingLevel (QStringView tagName)
		{
			if (tagName.size () != 2 || tagName [0] != 'h')
				return {};

			auto level = tagName [1].toLatin1 () - '0';
			if (level < 1 || level > 6)
				return {};
			return level;
		}

		BlockFormat WithClasses (const QList<QStringView>& classes, BlockFormat fmt)
		{
			if (classes.isEmpty ())
				return fmt;

			if (classes.contains (u"poem"_qsv))
			{
				fmt.MarginLeft_ = 50;
				fmt.MarginRight_ = 50;
			}

			if (classes.contains (u"stanza"_qsv))
			{
				fmt.MarginTop_ = 10;
				fmt.MarginBottom_ = 10;
			}

			if (classes.contains (u"epigraph"_qsv))
				fmt.Align_ = Qt::AlignRight;

			if (classes.contains (u"subscript"_qsv))
				fmt.Align_ = Qt::AlignRight;

			return fmt;
		}

		CharFormat WithClasses (const QList<QStringView>& classes, CharFormat fmt)
		{
			if (classes.isEmpty ())
				return fmt;

			if (classes.contains (u"poem"_qsv))
				fmt.IsItalic_ = true;

			if (classes.contains (u"epigraph"_qsv))
				fmt.IsItalic_ = true;

			if (classes.contains (u"emphasis"_qsv))
				fmt.IsItalic_ = true;

			if (classes.contains (u"subscript"_qsv))
				fmt.IsItalic_ = true;

			return fmt;
		}
	}

	BlockFormat TextDocumentFormatConfig::GetBlockFormat (const StylingContext& ctx) const
	{
		return WithClasses (ctx.Elem_.Classes_, GetDefaultTagBlockFormat (ctx.Elem_.Tag_));
	}

	std::optional<CharFormat> TextDocumentFormatConfig::GetCharFormat (const StylingContext& ctx) const
	{
		return WithClasses (ctx.Elem_.Classes_, GetDefaultTagCharFormat (ctx.Elem_.Tag_));
	}

	BlockFormat TextDocumentFormatConfig::GetDefaultTagBlockFormat (QStringView tagName) const
	{
		if (tagName == u"p"_qsv)
			return FormatP_;

		if (const auto& heading = GetHeadingLevel (tagName))
		{
			BlockFormat bf;
			bf.HeadingLevel_ = *heading;
			if (*heading <= 4)
				bf.Align_ = Qt::AlignHCenter;
			return { bf };
		}

		if (tagName == u"blockquote"_qsv)
			return { .MarginLeft_ = 50, .MarginRight_ = 50 };

		return {};
	}

	CharFormat TextDocumentFormatConfig::GetDefaultTagCharFormat (QStringView tagName) const
	{
		if (const auto& heading = GetHeadingLevel (tagName))
		{
			const auto& defFont = XSM_->property ("DefaultFont").value<QFont> ();
			return
			{
				.PointSize_ = defFont.pointSize () * (2.5 - (*heading - 1) / 3.),
				.IsBold_ = QFont::Weight::DemiBold,
				.Foreground_ = Palette_.Foreground_,
			};
		}

		if (tagName == u"b"_qsv || tagName == u"strong"_qsv)
			return { .IsBold_ = QFont::Weight::DemiBold, .Foreground_ = Palette_.Foreground_ };
		if (tagName == u"em"_qsv || tagName == u"i"_qsv)
			return { .IsItalic_ = true, .Foreground_ = Palette_.Foreground_ };
		if (tagName == u"s"_qsv)
			return { .IsStrikeThrough_ = true, .Foreground_ = Palette_.Foreground_ };
		if (tagName == u"sup"_qsv)
			return { .VerticalAlignment_ = QTextCharFormat::AlignSuperScript, .Foreground_ = Palette_.Foreground_ };
		if (tagName == u"sub"_qsv)
			return { .VerticalAlignment_ = QTextCharFormat::AlignSubScript, .Foreground_ = Palette_.Foreground_ };
		if (tagName == u"a"_qsv)
			return { .IsUnderline_ = true, .Foreground_ = Palette_.Link_ };

		return { .Foreground_ = Palette_.Foreground_ };
	}
}
