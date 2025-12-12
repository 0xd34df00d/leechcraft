/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "corochanneltest.h"
#include <QtConcurrentRun>
#include <QtTest>
#include "coro.h"
#include "coro/channel.h"
#include "coro/getresult.h"

QTEST_GUILESS_MAIN (LC::Util::CoroChannelTest)

namespace LC::Util
{
	void CoroChannelTest::testSingleRecv ()
	{
		Channel<int> ch;

		using namespace std::chrono_literals;

		constexpr auto producersCount = 32;
		constexpr auto repCount = 100;
		constexpr auto sleepLength = 1ms;
		std::vector<std::thread> threads;
		std::atomic_int expected;
		for (int i = 0; i < producersCount; ++i)
			threads.emplace_back ([&, i]
					{
						for (int j = 0; j < repCount; ++j)
						{
							const auto val = j * producersCount + i;
							ch.Push (val);
							expected.fetch_add (val, std::memory_order::relaxed);
							std::this_thread::sleep_for (sleepLength);
						}
					});

		auto reader = [] (Channel<int> *ch) -> Task<int, ThreadSafetyExtension>
		{
			int sum = 0;
			while (auto next = co_await ch->Pop ())
				sum += *next;
			co_return sum;
		} (&ch);

		for (auto& thread : threads)
			thread.join ();

		ch.Close ();

		auto result = GetTaskResult (reader);
		QCOMPARE (result, expected);
	}

	void CoroChannelTest::testManyRecvs ()
	{
		Channel<int> ch;

		using namespace std::chrono_literals;

		constexpr auto producersCount = 32;
		constexpr auto consumersCount = 8;
		constexpr auto repCount = 100;
		constexpr auto sleepLength = 1ms;
		std::vector<std::thread> producers;
		std::atomic_int expected;
		for (int i = 0; i < producersCount; ++i)
			producers.emplace_back ([&, i]
					{
						for (int j = 0; j < repCount; ++j)
						{
							const auto val = j * producersCount + i;
							ch.Push (val);
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
					while (auto next = co_await ch->Pop ())
						sum += *next;
					co_return sum;
				} (&ch);
				sum.fetch_add (GetTaskResult (reader));
			});

		for (auto& thread : producers)
			thread.join ();

		ch.Close ();

		for (auto& thread : consumers)
			thread.join ();

		QCOMPARE (sum, expected);
	}
}
