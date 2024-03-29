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
#include <QTextCharFormat>
#include "monocleutilconfig.h"
#include "types.h"

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

	struct BlockFormat;
	struct CharFormat;

	struct StylingContext;

	class MONOCLE_UTIL_API TextDocumentFormatConfig : public QObject
	{
		TextDocumentPalette Palette_;

		BlockFormat FormatP_ { .Align_ = Qt::AlignJustify, .Indent_ = 15 };

		Util::BaseSettingsManager *XSM_ = nullptr;
	public:
		static TextDocumentFormatConfig& Instance ();

		const TextDocumentPalette& GetPalette () const;
		void FormatDocument (QTextDocument&) const;

		QTextFrameFormat GetBodyFrameFormat () const;

		BlockFormat GetBlockFormat (const StylingContext&) const;
		std::optional<CharFormat> GetCharFormat (const StylingContext&) const;

		void SetXSM (Util::BaseSettingsManager&);
	private:
		void UpdatePalette ();
		void UpdateParaBlockFormat ();

		BlockFormat GetDefaultTagBlockFormat (QStringView tagName) const;
		CharFormat GetDefaultTagCharFormat (QStringView tagName) const;
	};
}
