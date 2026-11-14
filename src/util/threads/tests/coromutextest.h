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
	class CoroMutexTest : public QObject
	{
		Q_OBJECT
	private slots:
		void testFreeMutexIsLockedRightAway ();
		void testSectionsDontOverlap ();
		void testUnlockDefersWakeup ();

		void testCancelledWaiterDoesntGetMutex ();
		void testCancelledHolderUnlocks ();
		void testCancelledWaiterHandsMutexBack ();
		void testOwnerDiesWithSectionsInFlight ();
	};
}
