/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "monadtest.h"
#include <QtTest>
#include <monad.h>
#include <typelist.h>

QTEST_MAIN (LC::Util::MonadTest)

namespace LC
{
namespace Util
{
	void MonadTest::testBoostOptionalReturn ()
	{
		const auto& pure = Return<std::optional> (2);
		QCOMPARE (pure, std::optional<int> { 2 });
	}

	void MonadTest::testBoostOptionalBind ()
	{
		const auto& pure = Return<std::optional> (2);
		const auto& result = Bind (pure, [] (int value) { return std::optional<int> { ++value }; });
		QCOMPARE (result, std::optional<int> { 3 });
	}

	void MonadTest::testBoostOptionalBindEmpty ()
	{
		const auto& result = Bind (std::optional<int> {}, [] (int value) { return std::optional<int> { ++value }; });
		QCOMPARE (result, std::optional<int> {});
	}

	void MonadTest::testBoostOptionalBindOperator ()
	{
		const auto& pure = Return<std::optional> (2);
		const auto& result = pure >> [] (int value) { return std::optional<int> { ++value }; };
		QCOMPARE (result, std::optional<int> { 3 });
	}

	void MonadTest::testBoostOptionalBindEmptyOperator ()
	{
		const auto& result = std::optional<int> {} >> [] (int value) { return std::optional<int> { ++value }; };
		QCOMPARE (result, std::optional<int> {});
	}

	void MonadTest::testBoostOptionalDo ()
	{
		const auto& result = Do (std::optional<int> { 2 },
				[] (int a) -> std::optional<int> { return a * 2; },
				[] (int a) -> std::optional<int> { return a + 1; },
				[] (int a) -> std::optional<int> { return a * 3; });
		QCOMPARE (result, std::optional<int> { 15 });
	}

	void MonadTest::testBoostOptionalDoEmpty ()
	{
		bool called = false;
		const auto& result = Do (std::optional<int> { 2 },
				[] (int a) -> std::optional<int> { return a * 2; },
				[] (int) -> std::optional<int> { return {}; },
				[&called] (int a) -> std::optional<int> { called = true; return a * 3; });

		QCOMPARE (result, std::optional<int> {});
		QCOMPARE (called, false);
	}

	void MonadTest::testCompatibilitySingle ()
	{
		constexpr auto result = detail::IsCompatibleMonad<
					Typelist<QString>,
					Typelist<QString>
				> ();
		QCOMPARE (result, true);
	}

	void MonadTest::testCompatibilitySingleDif ()
	{
		constexpr auto result = detail::IsCompatibleMonad<
					Typelist<QString>,
					Typelist<QByteArray>
				> ();
		QCOMPARE (result, true);
	}

	void MonadTest::testCompatibilityMulti ()
	{
		constexpr auto result = detail::IsCompatibleMonad<
					Typelist<int, float, QString>,
					Typelist<int, float, QString>
				> ();
		QCOMPARE (result, true);
	}

	void MonadTest::testCompatibilityMultiDifEnd ()
	{
		constexpr auto result = detail::IsCompatibleMonad<
					Typelist<int, float, QString>,
					Typelist<int, float, QByteArray>
				> ();
		QCOMPARE (result, true);
	}

	void MonadTest::testInCompatibilityMulti ()
	{
		constexpr auto result = detail::IsCompatibleMonad<
					Typelist<int, float, QString>,
					Typelist<int, double, QString>
				> ();
		QCOMPARE (result, false);
	}

	void MonadTest::testInCompatibilityMultiStart ()
	{
		constexpr auto result = detail::IsCompatibleMonad<
					Typelist<int, float, QString>,
					Typelist<QByteArray, float, QString>
				> ();
		QCOMPARE (result, false);
	}
}
}
