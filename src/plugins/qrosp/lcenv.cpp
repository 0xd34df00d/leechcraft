/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lcenv.h"

namespace LC
{
namespace Qrosp
{
	LCEnv::LCEnv (QObject *parent)
	: QObject { parent }
	{
	}

	int LCEnv::GetMajorQtVersion () const
	{
		return QT_VERSION >> 16;
	}
}
}
