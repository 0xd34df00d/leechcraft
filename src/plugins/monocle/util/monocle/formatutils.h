/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

class QTextBlockFormat;
class QTextCharFormat;
class QTextFrameFormat;

namespace LC::Monocle
{
	struct BlockFormat;
	struct CharFormat;

	template<typename T>
	void SetBlockConfig (T& marginsFmt, QTextBlockFormat& blockFmt, const BlockFormat& blockCfg);

	extern template void SetBlockConfig<QTextFrameFormat> (QTextFrameFormat&, QTextBlockFormat&, const BlockFormat&);
	extern template void SetBlockConfig<QTextBlockFormat> (QTextBlockFormat&, QTextBlockFormat&, const BlockFormat&);

	void SetCharConfig (QTextCharFormat&, const CharFormat&);
}
