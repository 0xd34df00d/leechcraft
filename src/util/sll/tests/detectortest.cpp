/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "detectortest.h"
#include <QtTest>
#include <detector.h>

QTEST_MAIN (LC::Util::DetectorTest)

namespace LC
{
namespace Util
{
	template<typename T>
	using DoSmthDetector = decltype (std::declval<T> ().DoSmth (QString {}));

	void DetectorTest::testDetectMember ()
	{
		struct Foo
		{
			int DoSmth (const QString&);
		};

		struct Bar
		{
			void DoSmth (int);
		};

		static_assert (IsDetected_v<DoSmthDetector, Foo>);
		static_assert (!IsDetected_v<DoSmthDetector, Bar>);
	}
}
}
