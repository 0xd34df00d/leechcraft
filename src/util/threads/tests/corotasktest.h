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
		void testMoveOnlyReturn ();
		void testWait ();
		void testTaskDestr ();

		void testNetworkReplyGoodNoWait ();
		void testNetworkReplyGoodWait ();
		void testNetworkReplyBadNoWait ();
		void testNetworkReplyBadWait ();

		void testFutureAwaiter ();

		void testWaitMany ();
		void testWaitManyTuple ();
		void testWaitManyInvoking ();
		void testSharedTaskManyAwaiters ();
		void testSharedTaskAwaiterRemovedOnOuterDestruction ();
		void testSharedTaskExceptionManyAwaiters ();
		void testSharedTaskLastCopyDestroyedByAwaiter ();
		void testSharedTaskNonTriviallyMovableReturn ();

		void testSharedContextTaskManyAwaitersContextAlive ();
		void testSharedContextTaskContextDeadFansOutToAll ();
		void testSharedContextTaskBodyExceptionFansOut ();
		void testSharedContextTaskContextDeadDoesntWaitLong ();
		void testSharedContextTaskMixedAwaitersOwnContextDies ();

		void testEither ();
		void testEitherIgnoreLeft ();

		void testThrottleSameCoro ();
		void testThrottleSameCoroSlow ();
		void testThrottleSameCoroVerySlow ();
		void testThrottleManyCoros ();

		void testContextDestrBeforeFinish ();
		void testContextDestrAfterFinish ();
		void testContextDestrAwaitedSubtask ();
		void testContextDestrDoesntWaitTimer ();
		void testContextDestrDoesntWaitNetwork ();
		void testContextDestrDoesntWaitProcess ();
		void testContextDestrDoesntWaitFuture ();

#ifdef QT_DBUS_LIB
		void testDBus ();
#endif

		void cleanupTestCase ();
	};
}
