/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

namespace LC::Util
{
	class CoroTaskTest : public QObject
	{
		Q_OBJECT
	private slots:
		void testReturn ();
		void testWait ();
		void testTaskDestr ();

		void testNetworkReplyGoodNoWait ();
		void testNetworkReplyGoodWait ();
		void testNetworkReplyBadNoWait ();
		void testNetworkReplyBadWait ();

		void testFutureAwaiter ();

		void testWaitMany ();
		void testWaitManyTuple ();

		void testEither ();

		void testThrottleSameCoro ();
		void testThrottleSameCoroSlow ();
		void testThrottleManyCoros ();

		void testContextDestrBeforeFinish ();
		void testContextDestrAfterFinish ();
		void testContextDestrDoesntWaitTimer ();

		void cleanupTestCase ();
	};
}
