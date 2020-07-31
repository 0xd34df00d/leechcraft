/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "monadplustest.h"

#include "monadtest.h"
#include <QtTest>
#include <monadplus.h>
#include <lazy.h>
#include <typelist.h>

QTEST_MAIN (LC::Util::MonadPlusTest)

namespace LC
{
namespace Util
{
	void MonadPlusTest::testBoostOptionalMplus ()
	{
		const boost::optional<int> val1 { 1 };
		const boost::optional<int> val2 { 2 };
		const auto nothing = Mzero<boost::optional<int>> ();

		const auto res1 = val1 + val2;
		const auto res2 = val1 + nothing;
		const auto res3 = nothing + val1;
		const auto res4 = nothing + nothing;

		QCOMPARE (res1, val1);
		QCOMPARE (res2, val1);
		QCOMPARE (res3, val1);
		QCOMPARE (res4, nothing);
	}

	void MonadPlusTest::testBoostOptionalMsum ()
	{
		const boost::optional<int> val1 { 1 };
		const boost::optional<int> val2 { 2 };
		const boost::optional<int> val3 { 3 };
		const auto nothing = Mzero<boost::optional<int>> ();

		const auto res1 = Msum ({ val1, val2, val3 });
		const auto res2 = Msum ({ val1, nothing });
		const auto res3 = Msum ({ nothing, val1 });
		const auto res4 = Msum ({ nothing, nothing });
		const auto res5 = Msum ({ nothing });

		QCOMPARE (res1, val1);
		QCOMPARE (res2, val1);
		QCOMPARE (res3, val1);
		QCOMPARE (res4, nothing);
		QCOMPARE (res5, nothing);
	}

	void MonadPlusTest::testLazyBoostOptionalMsum ()
	{
		const auto val1 = MakeLazy (boost::optional<int> { 1 });
		const auto val2 = MakeLazy (boost::optional<int> { 2 });
		const auto val3 = MakeLazy (boost::optional<int> { 3 });
		const auto nothing = MakeLazy (Mzero<boost::optional<int>> ());

		const auto res1 = Msum ({ val1, val2, val3 });
		const auto res2 = Msum ({ val1, nothing });
		const auto res3 = Msum ({ nothing, val1 });
		const auto res4 = Msum ({ nothing, nothing });
		const auto res5 = Msum ({ nothing });

		QCOMPARE (res1 (), val1 ());
		QCOMPARE (res2 (), val1 ());
		QCOMPARE (res3 (), val1 ());
		QCOMPARE (res4 (), nothing ());
		QCOMPARE (res5 (), nothing ());
	}
}
}
