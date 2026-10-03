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
	class CoroChannelTest : public QObject
	{
		Q_OBJECT
	public:
		enum class Event
		{
			Sent,
			Closed,
			Received,
			ReceivedEnd,
		};
		Q_ENUM (Event)
	private slots:
		void testSingleRecv ();
		void testManyRecvs ();

		void testSingleThreaded ();
		void testSingleThreadedTimered ();

		void testMerge ();

		void testSendDefersWakeup ();
		void testCloseDefersWakeup ();
		void testCancelledReceiverDoesntTakeValue ();
		void testCancelledReceiverDoesntStrandValueBehindClose ();
		void testReceiverMayDestroyProducerOnSend ();
		void testReceiverMayDestroyProducerOnClose ();
	};
}
