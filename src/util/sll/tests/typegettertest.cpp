/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "typegettertest.h"
#include <type_traits>
#include <QtTest>
#include <typegetter.h>

QTEST_MAIN (LC::Util::TypeGetterTest)

namespace LC
{
namespace Util
{
	namespace
	{
		template<typename T>
		void PrintType ()
		{
			qDebug () << Q_FUNC_INFO;
		}
	}

	void TypeGetterTest::testArgType ()
	{
		const auto f = [] (int, const double) {};
		static_assert (std::is_same_v<ArgType_t<decltype (f), 0>, int>);
		static_assert (std::is_same_v<ArgType_t<decltype (f), 1>, double>);
	}

	void TypeGetterTest::testArgTypeRef ()
	{
		const auto f = [] (int&, const double&) {};
		static_assert (std::is_same_v<ArgType_t<decltype (f), 0>, int&>);
		static_assert (std::is_same_v<ArgType_t<decltype (f), 1>, const double&>);
	}

	void TypeGetterTest::testArgTypeRvalueRef ()
	{
		const auto f = [] (int&&, const double&&) {};
		static_assert (std::is_same_v<ArgType_t<decltype (f), 0>, int&&>);
		static_assert (std::is_same_v<ArgType_t<decltype (f), 1>, const double&&>);
	}

	void TypeGetterTest::testRetType ()
	{
		const auto f = [] (int val, const double) { return val; };
		static_assert (std::is_same_v<RetType_t<decltype (f)>, int>);
	}

	void TypeGetterTest::testRetTypeVoid ()
	{
		const auto f = [] {};
		static_assert (std::is_same_v<RetType_t<decltype (f)>, void>);
	}

	void TypeGetterTest::testRetTypeRef ()
	{
		int x;
		const auto f = [&x] (int, const double) -> int& { return x; };
		static_assert (std::is_same_v<RetType_t<decltype (f)>, int&>);
	}

	void TypeGetterTest::testRetTypeConstRef ()
	{
		int x;
		const auto f = [&x] (int, const double) -> const int& { return x; };
		static_assert (std::is_same_v<RetType_t<decltype (f)>, const int&>);
	}

	void TypeGetterTest::testArgCount ()
	{
		const auto f = [] (int, const double) {};
		static_assert (ArgCount_v<decltype (f)> == 2);
	}
}
}
