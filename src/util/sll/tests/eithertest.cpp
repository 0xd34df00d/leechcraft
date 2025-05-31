/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "eithertest.h"
#include <QtTest>
#include <either.h>
#include <void.h>
#include <functor.h>

QTEST_MAIN (LC::Util::EitherTest)

namespace LC
{
namespace Util
{
	using SomeEither_t = Either<int, QString>;

	void EitherTest::testBasicLeft ()
	{
		const auto& left = SomeEither_t::Left (1);
		QCOMPARE (left.IsLeft (), true);
		QCOMPARE (left.IsRight (), false);
		QCOMPARE (left.GetLeft (), 1);

		bool hadCaught = false;
		try
		{
			left.GetRight ();
		}
		catch (const std::exception&)
		{
			hadCaught = true;
		}
		QCOMPARE (hadCaught, true);
	}

	void EitherTest::testBasicRight ()
	{
		const auto& right = SomeEither_t::Right ("foo");
		QCOMPARE (right.IsLeft (), false);
		QCOMPARE (right.IsRight (), true);
		QCOMPARE (right.GetRight (), QString { "foo" });

		bool hadCaught = false;
		try
		{
			right.GetLeft ();
		}
		catch (const std::exception&)
		{
			hadCaught = true;
		}
		QCOMPARE (hadCaught, true);
	}
}
}
