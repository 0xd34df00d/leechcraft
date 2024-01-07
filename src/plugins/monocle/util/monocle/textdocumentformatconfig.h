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

	struct BlockFormat;
	struct CharFormat;

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

		BlockFormat GetDefaultTagBlockFormat (QStringView tagName) const;
		CharFormat GetDefaultTagCharFormat (QStringView tagName) const;
	};
}
