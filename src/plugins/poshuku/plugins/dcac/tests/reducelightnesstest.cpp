/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "reducelightnesstest.h"
#include <QtTest>
#include "../reducelightness.cpp"

QTEST_APPLESS_MAIN (LC::Poshuku::DCAC::ReduceLightnessTest)

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	void ReduceLightnessTest::testSSSE3 ()
	{
#ifdef SSE_ENABLED
		CHECKFEATURE (SSSE3)

		for (const auto& image : TestImages_)
		{
			const auto diff = CompareModifying (image,
					&ReduceLightnessDefault, &ReduceLightnessSSSE3, 1.5);
			QVERIFY2 (diff <= 1, "too big difference");
		}
#endif
	}

	void ReduceLightnessTest::testAVX2 ()
	{
#ifdef SSE_ENABLED
		CHECKFEATURE (AVX2)

		for (const auto& image : TestImages_)
		{
			const auto diff = CompareModifying (image,
					&ReduceLightnessDefault, &ReduceLightnessAVX2, 1.5);
			QVERIFY2 (diff <= 1, "too big difference");
		}
#endif
	}

	void ReduceLightnessTest::benchDefault ()
	{
		BenchmarkFunction ([] (QImage& img) { ReduceLightnessDefault (img, 1.5); });
	}

	void ReduceLightnessTest::benchSSSE3 ()
	{
#ifdef SSE_ENABLED
		CHECKFEATURE (SSSE3)
		BenchmarkFunction ([] (QImage& img) { ReduceLightnessSSSE3 (img, 1.5); });
#endif
	}

	void ReduceLightnessTest::benchAVX2 ()
	{
#ifdef SSE_ENABLED
		CHECKFEATURE (AVX2)
		BenchmarkFunction ([] (QImage& img) { ReduceLightnessAVX2 (img, 1.5); });
#endif
	}
}
}
}
