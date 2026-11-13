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
	class CoroCallbackTest : public QObject
	{
		Q_OBJECT
	private slots:
		void testNoArgs ();
		void testOneArg ();
		void testTwoArgsYieldTuple ();
		void testRefParameterYieldsOwnedValue ();
		void testMoveOnlyArgs ();
		void testAwaitedFromContextTask ();

		void testInvokedBeforeAwait ();
		void testResumesAfterCallbackReturns ();
		void testContinuationMayDestroyCallee ();
		void testSecondInvocationIsIgnored ();
		void testInvocationAfterCompletion ();
		void testSecondGetCallbackThrows ();

		void testCopiesAreNotDrops ();
		void testDroppedYieldsNullopt ();
		void testDroppedBeforeAwait ();
		void testInvocationBeforeDropKeepsValue ();

		void testContextDeathDetaches ();
		void testContextDeathAfterInvocation ();

		void testOuterCallbackDoesntResumeCancelledAwaiter_data ();
		void testOuterCallbackDoesntResumeCancelledAwaiter ();
		void testCancelledChildLeavesResultToParent ();
		void testCallbackDestroyedWhileAwaited ();
	};
}
