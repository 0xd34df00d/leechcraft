/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "formatutils.h"
#include <QTextFormat>
#include "types.h"

namespace LC::Monocle
{
	template<typename T>
	void SetBlockConfig (T& marginsFmt, QTextBlockFormat& blockFmt, const BlockFormat& blockCfg)
	{
		const auto set = [] (auto& fmt, auto setter, const auto& maybeVal)
		{
			if (maybeVal)
				(fmt.*setter) (*maybeVal);
		};
		set (marginsFmt, &T::setLeftMargin, blockCfg.MarginLeft_);
		set (marginsFmt, &T::setTopMargin, blockCfg.MarginTop_);
		set (marginsFmt, &T::setRightMargin, blockCfg.MarginRight_);
		set (marginsFmt, &T::setBottomMargin, blockCfg.MarginBottom_);
		set (blockFmt, &QTextBlockFormat::setAlignment, blockCfg.Align_);
		set (blockFmt, &QTextBlockFormat::setTextIndent, blockCfg.Indent_);
		set (blockFmt, &QTextBlockFormat::setHeadingLevel, blockCfg.HeadingLevel_);
		set (blockFmt, &QTextBlockFormat::setForeground, blockCfg.Foreground_);
		set (blockFmt, &QTextBlockFormat::setBackground, blockCfg.Background_);
	}

	template void SetBlockConfig<QTextFrameFormat> (QTextFrameFormat&, QTextBlockFormat&, const BlockFormat&);
	template void SetBlockConfig<QTextBlockFormat> (QTextBlockFormat&, QTextBlockFormat&, const BlockFormat&);

	void SetCharConfig (QTextCharFormat& fmt, const CharFormat& charCfg)
	{
		const auto set = [&fmt] (auto setter, const auto& maybeVal)
		{
			if (maybeVal)
				(fmt.*setter) (*maybeVal);
		};
		set (&QTextCharFormat::setFontPointSize, charCfg.PointSize_);
		set (&QTextCharFormat::setFontWeight, charCfg.IsBold_);
		set (&QTextCharFormat::setFontItalic, charCfg.IsItalic_);
		set (&QTextCharFormat::setFontUnderline, charCfg.IsUnderline_);
		set (&QTextCharFormat::setFontStrikeOut, charCfg.IsStrikeThrough_);
		set (&QTextCharFormat::setVerticalAlignment, charCfg.VerticalAlignment_);
		set (&QTextCharFormat::setForeground, charCfg.Foreground_);
		set (&QTextCharFormat::setBackground, charCfg.Background_);
	}
}
