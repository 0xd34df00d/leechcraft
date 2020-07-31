/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "colortemptest.h"
#include <QtTest>
#include "../colortemp.cpp"

QTEST_APPLESS_MAIN (LC::Poshuku::DCAC::ColorTempTest)

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	void ColorTempTest::testSSSE3 ()
	{
#ifdef SSE_ENABLED
		CHECKFEATURE (SSSE3)

		for (const auto& image : TestImages_)
		{
			const auto diff = CompareModifying (image,
					&AdjustColorTempDefault, &AdjustColorTempSSSE3, 6000);
			QVERIFY2 (diff <= 1, ("too big difference: " + std::to_string (diff)).c_str ());
		}
#endif
	}

	void ColorTempTest::testAVX2 ()
	{
#ifdef SSE_ENABLED
		CHECKFEATURE (AVX2)

		for (const auto& image : TestImages_)
		{
			const auto diff = CompareModifying (image,
					&AdjustColorTempDefault, &AdjustColorTempAVX2, 6000);
			QVERIFY2 (diff <= 1, ("too big difference: " + std::to_string (diff)).c_str ());
		}
#endif
	}

	void ColorTempTest::benchDefault ()
	{
		BenchmarkFunction ([] (QImage& image) { AdjustColorTempDefault (image, 6000); });
	}

	void ColorTempTest::benchSSSE3 ()
	{
#ifdef SSE_ENABLED
		CHECKFEATURE (SSSE3)

		BenchmarkFunction ([] (QImage& image) { AdjustColorTempSSSE3 (image, 6000); });
#endif
	}

	void ColorTempTest::benchAVX2 ()
	{
#ifdef SSE_ENABLED
		CHECKFEATURE (AVX2)

		BenchmarkFunction ([] (QImage& image) { AdjustColorTempAVX2 (image, 6000); });
#endif
	}
}
}
}
