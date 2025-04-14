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
#include <monad.h>

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

	void EitherTest::testFMapLeft ()
	{
		const auto& left = SomeEither_t::Left (1);
		const auto& fmapped = Fmap (left, [] (const QString& str) { return str + "_mapped"; });
		QCOMPARE (fmapped, left);
	}

	void EitherTest::testFMapRight ()
	{
		const auto& right = SomeEither_t::Right ("foo");
		const auto& fmapped = Fmap (right, [] (const QString& str) { return str + "_mapped"; });
		QCOMPARE (fmapped.GetRight (), QString { "foo_mapped" });
	}

	void EitherTest::testFMapRightChangeType ()
	{
		const auto& right = SomeEither_t::Right ("foo");
		const auto& fmapped = Fmap (right, [] (const QString& str) { return static_cast<long> (str.size ()); });
		QCOMPARE (fmapped.GetRight (), static_cast<long> (right.GetRight ().size ()));
	}

	void EitherTest::testPure ()
	{
		const auto& pure = Pure<Either, int> (QString { "foo" });
		QCOMPARE (pure, SomeEither_t::Right ("foo"));
	}

	void EitherTest::testGSL ()
	{
		const auto& pure = Pure<Either, int> ([] (const QString& s) { return s + "_pure"; });
		const auto& app = pure * Pure<Either, int> (QString { "foo" });
		QCOMPARE (app, SomeEither_t::Right ("foo_pure"));
	}

	void EitherTest::testGSLLeft ()
	{
		const auto& pure = Pure<Either, int> ([] (const QString& s) { return s + "_pure"; });
		const auto& value = SomeEither_t::Left (2);
		const auto& app = pure * value;
		QCOMPARE (app, value);
	}

	void EitherTest::testBind ()
	{
		const auto& res = Return<Either, int> (QString { "foo" }) >>
				[] (const QString& right) { return SomeEither_t::Right (right + "_bound"); };
		QCOMPARE (res, SomeEither_t::Right ("foo_bound"));
	}

	void EitherTest::testBindLeft ()
	{
		const auto& value = SomeEither_t::Left (2);
		const auto& res = value >>
				[] (const QString& right) { return SomeEither_t::Right (right + "_bound"); };
		QCOMPARE (res, value);
	}

	struct NoDefaultCtor
	{
		NoDefaultCtor () = delete;
		NoDefaultCtor (const QString&)
		{
		}

		bool operator== (const NoDefaultCtor&) const
		{
			return true;
		}
	};

	void EitherTest::testBindLeftNotConstructed ()
	{
		const auto& value = Either<NoDefaultCtor, Void>::Right ({});
		const auto& expected = Either<NoDefaultCtor, int>::Right (5);
		const auto res = value >>
				[&expected] (const auto&) { return expected; };
		QCOMPARE (res, expected);
	}
}
}
