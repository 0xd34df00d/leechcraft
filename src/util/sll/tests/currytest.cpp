/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "currytest.h"
#include <memory>
#include <QtTest>
#include <curry.h>

QTEST_APPLESS_MAIN (LC::Util::CurryTest)

namespace LC::Util
{
	void CurryTest::testBasic ()
	{
		auto sum = [] (int a, int b, int c) { return a + b + c; };
		QCOMPARE (Curry (sum) (1) (2) (3), 6);

		auto stored = Curry (sum);
		QCOMPARE (stored (1) (2) (3), 6);
		QCOMPARE (stored (0) (2) (3), 5);

		auto storedApplied = Curry (sum) (0);
		QCOMPARE (storedApplied (2) (3), 5);
	}

	void CurryTest::testMoveArgs ()
	{
		auto func = [] (std::unique_ptr<int> a, std::unique_ptr<int> b) { return *a + *b; };
		QCOMPARE (Curry (func) (std::make_unique<int> (1)) (std::make_unique<int> (2)), 3);

		auto curried = Curry (func) (std::make_unique<int> (1));
		QCOMPARE (std::move (curried) (std::make_unique<int> (2)), 3);
	}

	void CurryTest::testMoveFun ()
	{
		auto ptr = std::make_unique<int> (10);
		auto func = [ptr = std::move (ptr)] (std::unique_ptr<int> a, std::unique_ptr<int> b) { return *ptr +  *a + *b; };
		QCOMPARE (Curry (std::move (func)) (std::make_unique<int> (1)) (std::make_unique<int> (2)), 13);
	}

	void CurryTest::testRValueFun ()
	{
		auto sum = [] (int&& a, int&& b, int&& c) { return a + b + c; };
		QCOMPARE (Curry (sum) (1) (2) (3), 6);
	}

	void CurryTest::testRefModifications ()
	{
		int a = 5;
		int b = 6;
		auto func = [] (int& a, int& b) { ++a; ++b; return a + b; };
		QCOMPARE (Curry (func) (a) (b), 13);
		QCOMPARE (a, 6);
		QCOMPARE (b, 7);
	}

	namespace
	{
		template<typename T>
		struct Counter
		{
			static inline int DefConstrs_ = 0;
			static inline int CopyConstrs_ = 0;
			static inline int CopyAssignments_ = 0;
			static inline int MoveConstrs_ = 0;
			static inline int MoveAssignments_ = 0;
			static inline int Dtors_ = 0;

			Counter ()
			{
				++DefConstrs_;
			}

			Counter (const Counter&)
			{
				++CopyConstrs_;
			}

			Counter (Counter&&)
			{
				++MoveConstrs_;
			}

			Counter& operator= (const Counter&)
			{
				++CopyAssignments_;
				return *this;
			}

			Counter& operator= (Counter&&)
			{
				++MoveAssignments_;
				return *this;
			}

			~Counter ()
			{
				++Dtors_;
			}
		};
	}

	void CurryTest::testNoExtraCopiesByValue ()
	{
		using C1 = Counter<struct Tag1>;
		using C2 = Counter<struct Tag2>;

		auto func = [] (C1, C2) { return 0; };
		QCOMPARE (Curry (func) (C1 {}) (C2 {}), 0);

		QCOMPARE (C1::CopyConstrs_, 0);
		QCOMPARE (C2::CopyConstrs_, 0);
		QCOMPARE (C1::CopyAssignments_, 0);
		QCOMPARE (C2::CopyAssignments_, 0);

		QCOMPARE (C1::MoveConstrs_, 2);
		QCOMPARE (C2::MoveConstrs_, 1);
		QCOMPARE (C1::MoveAssignments_, 0);
		QCOMPARE (C2::MoveAssignments_, 0);
	}

	void CurryTest::testNoExtraCopiesByRef ()
	{
		using C1 = Counter<struct Tag1>;
		using C2 = Counter<struct Tag2>;

		auto func = [] (C1&&, C2&&) { return 0; };
		QCOMPARE (Curry (func) (C1 {}) (C2 {}), 0);

		QCOMPARE (C1::CopyConstrs_, 0);
		QCOMPARE (C2::CopyConstrs_, 0);
		QCOMPARE (C1::CopyAssignments_, 0);
		QCOMPARE (C2::CopyAssignments_, 0);

		QCOMPARE (C1::MoveConstrs_, 1);
		QCOMPARE (C2::MoveConstrs_, 0);
		QCOMPARE (C1::MoveAssignments_, 0);
		QCOMPARE (C2::MoveAssignments_, 0);
	}

	void CurryTest::testNoExtraCopiesByConstRef ()
	{
		using C1 = Counter<struct Tag1>;
		using C2 = Counter<struct Tag2>;

		auto func = [] (const C1&, const C2&) { return 0; };
		QCOMPARE (Curry (func) (C1 {}) (C2 {}), 0);

		QCOMPARE (C1::CopyConstrs_, 0);
		QCOMPARE (C2::CopyConstrs_, 0);
		QCOMPARE (C1::CopyAssignments_, 0);
		QCOMPARE (C2::CopyAssignments_, 0);

		QCOMPARE (C1::MoveConstrs_, 1);
		QCOMPARE (C2::MoveConstrs_, 0);
		QCOMPARE (C1::MoveAssignments_, 0);
		QCOMPARE (C2::MoveAssignments_, 0);
	}

	void CurryTest::testNoExtraCopiesByConstRefToExisting ()
	{
		using C1 = Counter<struct Tag1>;
		using C2 = Counter<struct Tag2>;

		auto func = [] (const C1&, const C2&) { return 0; };
		C1 c1;
		C2 c2;
		QCOMPARE (Curry (func) (c1) (c2), 0);

		QCOMPARE (C1::CopyConstrs_, 0);
		QCOMPARE (C2::CopyConstrs_, 0);
		QCOMPARE (C1::CopyAssignments_, 0);
		QCOMPARE (C2::CopyAssignments_, 0);

		QCOMPARE (C1::MoveConstrs_, 0);
		QCOMPARE (C2::MoveConstrs_, 0);
		QCOMPARE (C1::MoveAssignments_, 0);
		QCOMPARE (C2::MoveAssignments_, 0);
	}
}
