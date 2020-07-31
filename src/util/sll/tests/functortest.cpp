/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "functortest.h"
#include <QtTest>
#include <functor.h>

QTEST_MAIN (LC::Util::FunctorTest)

namespace LC
{
namespace Util
{
	void FunctorTest::testBoostOptionalFMap ()
	{
		boost::optional<int> value { 2 };
		const auto& fmapped = Fmap (value, [] (int val) { return QString::number (val); });
		QCOMPARE (boost::optional<QString> { "2" }, fmapped);
	}

	void FunctorTest::testBoostOptionalFMapEmpty ()
	{
		boost::optional<int> value;
		const auto& fmapped = Fmap (value, [] (int val) { return QString::number (val); });
		QCOMPARE (boost::optional<QString> {}, fmapped);
	}

	void FunctorTest::testIsFunctorTrue ()
	{
		static_assert (IsFunctor<boost::optional<int>> (), "test failed");
	}

	void FunctorTest::testIsFunctorFalse ()
	{
		static_assert (!IsFunctor<int> (), "test failed");
	}
}
}
