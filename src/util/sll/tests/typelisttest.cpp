/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "typelisttest.h"
#include <QtTest>
#include <typelist.h>
#include <typelevel.h>

QTEST_MAIN (LC::Util::TypelistTest)

namespace LC
{
namespace Util
{
	void TypelistTest::testHasTypeTrue ()
	{
		static_assert (HasType<struct Foo> (Typelist<struct Bar, struct Baz, struct Foo> {}), "test failed");
	}

	void TypelistTest::testHasTypeFalse ()
	{
		static_assert (!HasType<struct Foo> (Typelist<struct Bar, struct Baz, struct Qux> {}), "test failed");
	}

	template<typename T>
	using IsVoid_t = std::is_same<T, void>;

	void TypelistTest::testFilter ()
	{
		using List_t = Typelist<struct Foo, struct Bar, void, void, int, double, void>;
		using Expected_t = Typelist<struct Foo, struct Bar, int, double>;
		using Removed_t = Typelist<void, void, void>;

		static_assert (std::is_same<Removed_t, Filter_t<IsVoid_t, List_t>>::value, "test failed");
		static_assert (std::is_same<Expected_t, Filter_t<Not<IsVoid_t>::Result_t, List_t>>::value, "test failed");
	}
}
}
