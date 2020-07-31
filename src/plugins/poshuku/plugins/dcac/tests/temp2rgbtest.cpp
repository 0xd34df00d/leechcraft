/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "temp2rgbtest.h"
#include <QtTest>
#include "../colortemp.cpp"

QTEST_APPLESS_MAIN (LC::Poshuku::DCAC::Temp2RgbTest)

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	namespace
	{
		struct ColorPrinter
		{
			QRgb Rgb_;
			ColorPrinter (QRgb rgb)
			: Rgb_ { rgb }
			{
			}
		};

		bool operator== (const ColorPrinter& left, const ColorPrinter& right)
		{
			return left.Rgb_ == right.Rgb_;
		}

		char* toString (const ColorPrinter& rgb)
		{
			using QTest::toString;
			return toString ("RGB { " +
					QByteArray::number (qRed (rgb.Rgb_)) + ", " +
					QByteArray::number (qGreen (rgb.Rgb_)) + ", " +
					QByteArray::number (qBlue (rgb.Rgb_)) + "}");
		}
	}

	void Temp2RgbTest::testTemp2Rgb_data ()
	{
		QTest::addColumn<uint> ("temp");
		QTest::addColumn<uint> ("result");

#define ADD(x,r,g,b) QTest::newRow (#x) << static_cast<uint> (x) << qRgb(r,g,b);
		ADD (4000, 255, 206, 166);
		ADD (4500, 255, 218, 187);
		ADD (5000, 255, 228, 206);
		ADD (5500, 255, 237, 222);
		ADD (6000, 255, 246, 237);
		ADD (6500, 255, 254, 250);
		ADD (7000, 243, 242, 255);
#undef ADD
	}

	void Temp2RgbTest::testTemp2Rgb ()
	{
		QFETCH (uint, temp);
		QFETCH (uint, result);

		QCOMPARE (ColorPrinter { Temp2Rgb (temp) }, ColorPrinter { result });
	}
}
}
}
