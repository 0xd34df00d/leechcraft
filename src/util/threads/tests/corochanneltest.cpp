/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "corochanneltest.h"
#include <memory>
#include <QtConcurrentRun>
#include <QtTest>
#include "coro.h"
#include "coro/channel.h"
#include "coro/channelutils.h"
#include "coro/getresult.h"
#include "coro/threadsafety.h"

QTEST_GUILESS_MAIN (LC::Util::CoroChannelTest)

namespace LC::Util
{
	void CoroChannelTest::testSingleRecv ()
	{
		using namespace std::chrono_literals;

		constexpr auto producersCount = 32;
		constexpr auto repCount = 100;
		constexpr auto sleepLength = 1ms;

		Channel<int> ch;

		std::vector<std::thread> threads;
		std::atomic_int expected;
		for (int i = 0; i < producersCount; ++i)
			threads.emplace_back ([&, i]
					{
						for (int j = 0; j < repCount; ++j)
						{
							const auto val = j * producersCount + i;
							ch.Send (val);
							expected.fetch_add (val, std::memory_order::relaxed);
							std::this_thread::sleep_for (sleepLength);
						}
					});

		auto mainThread = std::this_thread::get_id ();
		auto reader = [] (auto mainThread, Channel<int> *ch) -> Task<int, ThreadSafetyExtension>
		{
			int sum = 0;
			while (auto next = co_await ch->Receive ())
			{
				[=]
				{
					auto thisThread = std::this_thread::get_id ();
					QCOMPARE (thisThread, mainThread);
				} ();

				sum += *next;
			}
			co_return sum;
		} (mainThread, &ch);

		for (auto& thread : threads)
			thread.join ();

		ch.Close ();

		auto result = GetTaskResult (reader);
		QCOMPARE (result, expected);
	}

	void CoroChannelTest::testManyRecvs ()
	{
		using namespace std::chrono_literals;

		constexpr auto producersCount = 32;
		constexpr auto consumersCount = 8;
		constexpr auto repCount = 100;
		constexpr auto sleepLength = 1ms;

		Channel<int> ch;

		std::vector<std::thread> producers;
		std::atomic_int expected;
		for (int i = 0; i < producersCount; ++i)
			producers.emplace_back ([&, i]
					{
						for (int j = 0; j < repCount; ++j)
						{
							const auto val = j * producersCount + i;
							ch.Send (val);
							expected.fetch_add (val, std::memory_order::relaxed);
							std::this_thread::sleep_for (sleepLength);
						}
					});

		std::atomic_int sum;
		std::vector<std::thread> consumers;
		for (int i = 0; i < consumersCount; ++i)
			consumers.emplace_back ([&]
			{
				auto reader = [] (Channel<int> *ch) -> Task<int, ThreadSafetyExtension>
				{
					int sum = 0;
					while (auto next = co_await ch->Receive ())
						sum += *next;
					co_return sum;
				} (&ch);
				sum.fetch_add (GetTaskResult (reader));
			});

		for (auto& producer : producers)
			producer.join ();

		ch.Close ();

		for (auto& consumer : consumers)
			consumer.join ();

		QCOMPARE (sum, expected);
	}

	void CoroChannelTest::testSingleThreaded ()
	{
		constexpr auto iterations = 1000;

		Channel<int> ch;

		auto reader = [] (Channel<int> *ch) -> Task<int, ThreadSafetyExtension>
		{
			int sum = 0;
			while (auto next = co_await ch->Receive ())
				sum += *next;
			co_return sum;
		} (&ch);

		int expected = 0;
		for (int i = 0; i < iterations; ++i)
		{
			expected += i;
			ch.Send (i);
		}

		ch.Close ();

		const auto result = GetTaskResult (reader);
		QCOMPARE (result, expected);
	}

	void CoroChannelTest::testSingleThreadedTimered ()
	{
		using namespace std::chrono_literals;

		constexpr auto iterations = 100;
		constexpr auto interval = 1ms;

		Channel<int> ch;

		auto reader = [] (Channel<int> *ch) -> Task<int, ThreadSafetyExtension>
		{
			int sum = 0;
			while (auto next = co_await ch->Receive ())
				sum += *next;
			co_return sum;
		} (&ch);

		int expected = 0;

		QTimer timer;
		timer.callOnTimeout ([&, i = 0] mutable
				{
					expected += i;
					ch.Send (i);
					if (++i == iterations)
					{
						timer.stop ();
						ch.Close ();
					}
				});
		timer.start (interval);

		const auto result = GetTaskResult (reader);
		QCOMPARE (result, expected);
	}

	void CoroChannelTest::testMerge ()
	{
		using namespace std::chrono_literals;

		constexpr auto numChannels = 100;
		constexpr auto iterations = 100;
		constexpr auto interval = 1ms;

		QVector<Channel_ptr<int>> channels;
		std::generate_n (std::back_inserter (channels), numChannels, [] { return std::make_shared<Channel<int>> (); });

		auto merged = MergeChannels (channels);

		auto reader = [] (Channel<int> *ch) -> Task<int, ThreadSafetyExtension>
		{
			int sum = 0;
			while (auto next = co_await ch->Receive ())
				sum += *next;
			co_return sum;
		} (merged.get ());

		int expected = 0;

		QTimer timer;
		timer.callOnTimeout ([&, i = 0] mutable
				{
					for (int j = 0; j < numChannels; ++j)
					{
						auto value = i * numChannels + j;
						expected += value;
						channels [j]->Send (value);
					}
					if (++i == iterations)
					{
						timer.stop ();
						for (auto chan : channels)
							chan->Close ();
					}
				});
		timer.start (interval);

		const auto result = GetTaskResult (reader);
		QCOMPARE (result, expected);
	}

	namespace
	{
		using Event = CoroChannelTest::Event;

		Task<void> Receiver (Channel<int>& ch, QList<Event>& log)
		{
			log << (co_await ch ? Event::Received : Event::ReceivedEnd);
		}
	}

	void CoroChannelTest::testSendDefersWakeup ()
	{
		Channel<int> ch;
		QList<Event> log;
		auto reader = Receiver (ch, log);
		ch.Send (42);
		log << Event::Sent;
		GetTaskResult (reader);
		QCOMPARE (log, (QList { Event::Sent, Event::Received }));
	}

	void CoroChannelTest::testCloseDefersWakeup ()
	{
		Channel<int> ch;
		QList<Event> log;
		auto reader = Receiver (ch, log);
		ch.Close ();
		log << Event::Closed;
		GetTaskResult (reader);
		QCOMPARE (log, (QList { Event::Closed, Event::ReceivedEnd }));
	}

	namespace
	{
		ContextTask<void> Doomed (CoroContext& context, Channel<int>& ch, QList<int>& received)
		{
			co_await AddContext { context };
			if (const auto value = co_await ch)
				received << *value;
		}

		Task<void> Survivor (Channel<int>& ch, QList<int>& received)
		{
			while (const auto value = co_await ch)
				received << *value;
		}
	}

	void CoroChannelTest::testCancelledReceiverDoesntTakeValue ()
	{
		Channel<int> ch;
		auto context = std::make_unique<CoroContext> ();
		QList<int> doomedReceived;
		auto doomed = Doomed (*context, ch, doomedReceived);
		ch.Send (42);
		context.reset ();

		QList<int> survivorReceived;
		auto survivor = Survivor (ch, survivorReceived);
		ch.Close ();

		QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (doomed));
		GetTaskResult (survivor);
		QVERIFY (doomedReceived.isEmpty ());
		QCOMPARE (survivorReceived, (QList { 42 }));
	}

	void CoroChannelTest::testCancelledReceiverDoesntStrandValueBehindClose ()
	{
		Channel<int> ch;
		auto context = std::make_unique<CoroContext> ();
		QList<int> doomedReceived;
		QList<int> survivorReceived;
		auto doomed = Doomed (*context, ch, doomedReceived);
		auto survivor = Survivor (ch, survivorReceived);
		ch.Send (42);
		ch.Close ();
		context.reset ();

		QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (doomed));
		GetTaskResult (survivor);
		QVERIFY (doomedReceived.isEmpty ());
		QCOMPARE (survivorReceived, (QList { 42 }));
	}

	namespace
	{
		struct Producer : QObject
		{
			Channel<int> Ch_;
			bool InCall_ = false;

			ContextTask<void> SendOne ()
			{
				co_await AddContext { CoroContext_ };
				InCall_ = true;
				Ch_.Send (42);
				InCall_ = false;
			}

			ContextTask<void> CloseChannel ()
			{
				co_await AddContext { CoroContext_ };
				InCall_ = true;
				Ch_.Close ();
				InCall_ = false;
			}

			LC_CORO_CONTEXT
		};

		Task<bool> DestroyingReceiver (std::unique_ptr<Producer>& producer)
		{
			co_await producer->Ch_;
			const auto inCall = producer->InCall_;
			if (!inCall)
				producer.reset ();
			co_return inCall;
		}
	}

	void CoroChannelTest::testReceiverMayDestroyProducerOnSend ()
	{
		auto producer = std::make_unique<Producer> ();
		auto reader = DestroyingReceiver (producer);
		producer->SendOne ();
		QVERIFY (!GetTaskResult (reader));
		QVERIFY (!producer);
	}

	void CoroChannelTest::testReceiverMayDestroyProducerOnClose ()
	{
		auto producer = std::make_unique<Producer> ();
		auto reader = DestroyingReceiver (producer);
		producer->CloseChannel ();
		QVERIFY (!GetTaskResult (reader));
		QVERIFY (!producer);
	}
}
