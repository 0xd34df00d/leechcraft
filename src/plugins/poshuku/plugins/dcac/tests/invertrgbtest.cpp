/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "invertrgbtest.h"

#include <QtTest>
#include "../invertcolors.cpp"

QTEST_APPLESS_MAIN (LC::Poshuku::DCAC::InvertRgbTest)

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	void InvertRgbTest::benchDefault ()
	{
		BenchmarkFunction (&InvertRgbDefault);
	}
}
}
}
