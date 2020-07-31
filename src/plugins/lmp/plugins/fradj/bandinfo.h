/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QList>

namespace LC
{
namespace LMP
{
namespace Fradj
{
	struct BandInfo
	{
		double Freq_;
		double Width_;

		BandInfo (double freq, double width = 0);
	};

	typedef QList<BandInfo> BandInfos_t;
}
}
}
