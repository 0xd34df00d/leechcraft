/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "testbase.h"
#include <QtConcurrentMap>
#include <random>

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	namespace
	{
		QImage GetRandomImage (const QSize& size)
		{
			std::mt19937 gen { std::random_device {} () };
			std::uniform_int_distribution<uint32_t> dist { 0xff000000, 0xffffffff };

			QImage image { size, QImage::Format_ARGB32 };
			for (int y = 0; y < size.height (); ++y)
			{
				const auto scanline = reinterpret_cast<QRgb*> (image.scanLine (y));
				for (int x = 0; x < size.width (); ++x)
					scanline [x] = dist (gen);
			}
			return image;
		}

		QList<QImage> GetRandomImages (const QSize& size, int count)
		{
			QVector<QImage> result;
			result.resize (count);
			QtConcurrent::blockingMap (result,
					[&size] (QImage& image) { image = GetRandomImage (size); });
			return result.toList ();
		}

		const auto RefTestCount = 5;

		const auto BenchImageCount = 5;
	}

	void TestBase::initTestCase ()
	{
		TestImages_ = GetRandomImages ({ 1920, 1080 }, RefTestCount);

		for (auto size : QList<QSize> { { 1440, 900 }, { 1920, 1080 }, { 2560, 1440 }, { 3840, 2160 } })
			BenchImages_ [size] = GetRandomImages (size, BenchImageCount);
	}
}
}
}
