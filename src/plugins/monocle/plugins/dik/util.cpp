/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QByteArray>

namespace LC
{
namespace Monocle
{
namespace Dik
{
	quint32 Read32 (const QByteArray& data, int offset)
	{
		quint32 result = 0;
		for (int i = 0; i < 4; ++i)
		{
			result <<= 8;
			result += static_cast<uchar> (data [offset + i]);
		}
		return result;
	}
}
}
}
