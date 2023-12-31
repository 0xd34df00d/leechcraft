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
#include <QMargins>
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

		/** @brief The link color.
		 */
		QColor Link_;
	};

	struct BlockFormat
	{
		Qt::AlignmentFlag Align_ = Qt::AlignLeft;

		/** Margins in px.
		 */
		QMargins Margins_ { 0, 0, 0, 0 };

		/** First line indent in px.
		 */
		int Indent_ = 0;

		/** Heading level (as in h1-h6).
		 */
		int HeadingLevel_ = 0;
	};

	struct CharFormat
	{
		qreal PointSize_;
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
