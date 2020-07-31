/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "getgraytest.h"
#include <QtTest>
#include "../invertcolors.cpp"

QTEST_APPLESS_MAIN (LC::Poshuku::DCAC::GetGrayTest)

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	void GetGrayTest::testSSSE3 ()
	{
#ifdef SSE_ENABLED
		CHECKFEATURE (SSSE3)

		for (const auto& image : TestImages_)
		{
			const auto ref = GetGrayDefault (image);
			const auto sse4 = GetGraySSSE3 (image);

			QCOMPARE (ref, sse4);
		}
#endif
	}

	void GetGrayTest::testAVX2 ()
	{
#ifdef SSE_ENABLED
		CHECKFEATURE (AVX2)

		for (const auto& image : TestImages_)
		{
			const auto ref = GetGrayDefault (image);
			const auto avx2 = GetGrayAVX2 (image);

			QCOMPARE (ref, avx2);
		}
#endif
	}

	void GetGrayTest::benchDefault ()
	{
		BenchmarkFunction (&GetGrayDefault);
	}

	void GetGrayTest::benchSSSE3 ()
	{
#ifdef SSE_ENABLED
		CHECKFEATURE (SSSE3)

		BenchmarkFunction (&GetGraySSSE3);
#endif
	}

	void GetGrayTest::benchAVX2 ()
	{
#ifdef SSE_ENABLED
		CHECKFEATURE (AVX2)

		BenchmarkFunction (&GetGrayAVX2);
#endif
	}
}
}
}
