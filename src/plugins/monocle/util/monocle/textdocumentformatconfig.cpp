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
#include <util/sll/qtutil.h>
#include <xmlsettingsdialog/basesettingsmanager.h>

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

	QTextFrameFormat TextDocumentFormatConfig::GetBodyFrameFormat () const
	{
		const auto margin = [this] (const QByteArray& orient) { return XSM_->property (orient + "Margin").toInt (); };
		QTextFrameFormat fmt;
		fmt.setLeftMargin (margin ("Left"));
		fmt.setRightMargin (margin ("Right"));
		fmt.setTopMargin (margin ("Top"));
		fmt.setBottomMargin (margin ("Bottom"));
		fmt.setWidth (XSM_->property ("PageWidth").toInt ());
		return fmt;
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

		BlockFormat GetDefaultTagBlockFormat (QStringView tagName, const Util::BaseSettingsManager&)
		{
			if (tagName == u"p"_qsv)
				return { .Align_ = Qt::AlignJustify, .Indent_ = 15 };

			if (const auto& heading = GetHeadingLevel (tagName))
			{
				BlockFormat bf;
				bf.HeadingLevel_ = *heading;
				if (*heading <= 2)
					bf.Align_ = Qt::AlignHCenter;
				return { bf };
			}

			if (tagName == u"blockquote"_qsv)
				return { .MarginLeft_ = 100 };

			return {};
		}

		CharFormat GetDefaultTagCharFormat (QStringView tagName, const Util::BaseSettingsManager& xsm)
		{
			if (const auto& heading = GetHeadingLevel (tagName))
			{
				const auto& defFont = xsm.property ("DefaultFont").value<QFont> ();
				CharFormat cf
				{
					.PointSize_ = defFont.pointSize () * (2. - *heading / 10.),
					.IsBold_ = QFont::Weight::DemiBold,
				};
				return cf;
			}

			if (tagName == u"b"_qsv || tagName == u"strong"_qsv)
				return CharFormat { .IsBold_ = QFont::Weight::DemiBold };
			if (tagName == u"em"_qsv || tagName == u"i"_qsv)
				return CharFormat { .IsItalic_ = true };
			if (tagName == u"s"_qsv)
				return CharFormat { .IsStrikeThrough_ = true };

			return {};
		}

		BlockFormat WithClasses (const QList<QStringView>& classes, BlockFormat fmt)
		{
			if (classes.isEmpty ())
				return fmt;

			if (classes.contains (u"poem"_qsv))
				fmt.MarginLeft_ = 100;

			if (classes.contains (u"stanza"_qsv))
			{
				fmt.MarginTop_ = 10;
				fmt.MarginBottom_ = 10;
			}

			if (classes.contains (u"epigraph"_qsv))
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

			return fmt;
		}
	}

	BlockFormat TextDocumentFormatConfig::GetBlockFormat (QStringView tagName, QStringView klass) const
	{
		return WithClasses (klass.split (' '), GetDefaultTagBlockFormat (tagName, *XSM_));
	}

	std::optional<CharFormat> TextDocumentFormatConfig::GetCharFormat (QStringView tagName, QStringView klass) const
	{
		return WithClasses (klass.split (' '), GetDefaultTagCharFormat (tagName, *XSM_));
	}

	void TextDocumentFormatConfig::SetXSM (Util::BaseSettingsManager& xsm)
	{
		XSM_ = &xsm;

		UpdatePalette ();
	}

	void TextDocumentFormatConfig::UpdatePalette ()
	{
		// TODO make configurable
		const auto& palette = qGuiApp->palette ();
		Palette_ =
		{
			.Background_ = palette.color (QPalette::Base),
			.Foreground_ = palette.color (QPalette::Text),
			.Link_ = palette.color (QPalette::Link),
		};
	}
}
