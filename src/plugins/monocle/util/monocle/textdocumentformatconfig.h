/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QColor>
#include <QFont>
#include <QMargins>
#include <QTextCharFormat>
#include "monocleutilconfig.h"

class QTextDocument;
class QTextFrameFormat;

namespace LC::Util
{
	class BaseSettingsManager;
}

namespace LC::Monocle
{
	struct TextDocumentPalette
	{
		/** @brief The default background color of the whole page.
		 */
		QColor Background_;

		/** @brief The default foreground color of the whole page.
		 */
		QColor Foreground_;

		/** @brief The link color.
		 */
		QColor Link_;
	};

	struct BlockFormat
	{
		std::optional<Qt::AlignmentFlag> Align_;

		/** Margins in px.
		 */
		std::optional<int> MarginLeft_;
		std::optional<int> MarginTop_;
		std::optional<int> MarginRight_;
		std::optional<int> MarginBottom_;

		/** First line indent in px.
		 */
		std::optional<int> Indent_;

		/** Heading level (as in h1-h6).
		 */
		std::optional<int> HeadingLevel_;

		std::optional<QBrush> Background_;
		std::optional<QBrush> Foreground_;
	};

	struct CharFormat
	{
		std::optional<qreal> PointSize_ {};

		std::optional<QFont::Weight> IsBold_ {};
		std::optional<bool> IsItalic_ {};
		std::optional<bool> IsUnderline_ {};
		std::optional<bool> IsStrikeThrough_ {};

		std::optional<QTextCharFormat::VerticalAlignment> VerticalAlignment_;

		std::optional<QBrush> Background_;
		std::optional<QBrush> Foreground_;
	};

	class MONOCLE_UTIL_API TextDocumentFormatConfig
	{
		TextDocumentPalette Palette_;

		Util::BaseSettingsManager *XSM_ = nullptr;
	public:
		static TextDocumentFormatConfig& Instance ();

		const TextDocumentPalette& GetPalette () const;
		void FormatDocument (QTextDocument&) const;

		QTextFrameFormat GetBodyFrameFormat () const;

		BlockFormat GetBlockFormat (QStringView tagName, QStringView klass) const;
		std::optional<CharFormat> GetCharFormat (QStringView tagName, QStringView klass) const;

		void SetXSM (Util::BaseSettingsManager&);
	private:
		void UpdatePalette ();
	};
}
