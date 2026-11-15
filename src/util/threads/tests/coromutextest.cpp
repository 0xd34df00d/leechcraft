/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "coromutextest.h"
#include <algorithm>
#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <utility>
#include <vector>
#include <QCoreApplication>
#include <QTest>
#include <util/sll/util.h>
#include <util/sll/void.h>
#include <util/test/randomwalk.h>
#include "coro.h"
#include "coro/channel.h"
#include "coro/coromutex.h"
#include "coro/getresult.h"

QTEST_GUILESS_MAIN (LC::Util::CoroMutexTest)

namespace LC::Util
{
	namespace
	{
		using Gate = Channel<Void>;

		void Open (Gate& gate)
		{
			gate.Send (Void {});
		}

		ContextTask<void> Section (CoroContext& context, CoroMutex& mutex, Gate& gate, QStringList& log, QString name)
		{
			co_await AddContext { context };
			{
				const auto guard = co_await mutex.Lock ();
				log << name + " in";
				co_await gate;
				log << name + " out";
			}
			log << name + (mutex.IsLocked () ? " handed over" : " unlocked");
		}

		ContextTask<void> Waiter (CoroContext& context, CoroMutex& mutex, QList<int>& order, int index)
		{
			co_await AddContext { context };
			const auto guard = co_await mutex.Lock ();
			order << index;
		}

		Task<void> CountedSection (CoroMutex& mutex, int& inside, int& maxInside, int& finished)
		{
			using namespace std::chrono_literals;

			const auto guard = co_await mutex.Lock ();
			maxInside = std::max (maxInside, ++inside);
			co_await 1ms;
			--inside;
			++finished;
		}
	}

	void CoroMutexTest::testFreeMutexIsLockedRightAway ()
	{
		CoroContext context;
		CoroMutex mutex;
		Gate gate;
		QStringList log;
		QVERIFY (!mutex.IsLocked ());

		auto section = Section (context, mutex, gate, log, "a");
		QCOMPARE (log, (QStringList { "a in" }));
		QVERIFY (mutex.IsLocked ());

		Open (gate);
		QTRY_COMPARE (log, (QStringList { "a in", "a out", "a unlocked" }));
		QVERIFY (!mutex.IsLocked ());
	}

	void CoroMutexTest::testSectionsDontOverlap ()
	{
		constexpr auto sectionsCount = 16;

		CoroMutex mutex;
		int inside = 0;
		int maxInside = 0;
		int finished = 0;
		std::vector<Task<void>> sections;
		for (int i = 0; i < sectionsCount; ++i)
			sections.push_back (CountedSection (mutex, inside, maxInside, finished));

		QTRY_COMPARE (finished, sectionsCount);
		QCOMPARE (maxInside, 1);
		QVERIFY (!mutex.IsLocked ());
	}

	void CoroMutexTest::testUnlockDefersWakeup ()
	{
		CoroContext context;
		CoroMutex mutex;
		Gate gate;
		QStringList log;
		auto first = Section (context, mutex, gate, log, "a");
		auto second = Section (context, mutex, gate, log, "b");
		QCOMPARE (log, (QStringList { "a in" }));

		Open (gate);
		Open (gate);
		QTRY_COMPARE (log, (QStringList { "a in", "a out", "a handed over", "b in", "b out", "b unlocked" }));
	}

	void CoroMutexTest::testWaitersGetMutexInOrder ()
	{
		constexpr auto waitersCount = 16;

		CoroContext context;
		CoroMutex mutex;
		Gate gate;
		QStringList log;
		auto holder = Section (context, mutex, gate, log, "holder");

		QList<int> order;
		QList<int> expected;
		std::vector<ContextTask<void>> waiters;
		for (int i = 0; i < waitersCount; ++i)
		{
			waiters.push_back (Waiter (context, mutex, order, i));
			expected << i;
		}

		Open (gate);
		QTRY_COMPARE (order, expected);
	}

	void CoroMutexTest::testCancelledWaiterDoesntGetMutex ()
	{
		CoroContext context;
		CoroMutex mutex;
		Gate gate;
		QStringList log;
		auto doomedContext = std::make_unique<CoroContext> ();
		auto holder = Section (context, mutex, gate, log, "holder");
		auto doomed = Section (*doomedContext, mutex, gate, log, "doomed");
		auto survivor = Section (context, mutex, gate, log, "survivor");
		doomedContext.reset ();

		Open (gate);
		Open (gate);
		QTRY_COMPARE (log, (QStringList
				{
					"holder in", "holder out", "holder handed over",
					"survivor in", "survivor out", "survivor unlocked"
				}));
		QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (doomed));
	}

	void CoroMutexTest::testCancelledHolderUnlocks ()
	{
		CoroContext context;
		CoroMutex mutex;
		Gate gate;
		QStringList log;
		auto doomedContext = std::make_unique<CoroContext> ();
		auto doomed = Section (*doomedContext, mutex, gate, log, "doomed");
		auto survivor = Section (context, mutex, gate, log, "survivor");
		doomedContext.reset ();

		Open (gate);
		QTRY_COMPARE (log, (QStringList { "doomed in", "survivor in", "survivor out", "survivor unlocked" }));
		QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (doomed));
	}

	void CoroMutexTest::testCancelledWaiterHandsMutexBack ()
	{
		CoroContext context;
		CoroMutex mutex;
		Gate gate;
		QStringList log;
		auto holderContext = std::make_unique<CoroContext> ();
		auto waiterContext = std::make_unique<CoroContext> ();
		auto holder = Section (*holderContext, mutex, gate, log, "holder");
		auto waiter = Section (*waiterContext, mutex, gate, log, "waiter");

		// the mutex is now on its way to the waiter, which is yet to be resumed
		holderContext.reset ();
		QVERIFY (mutex.IsLocked ());

		waiterContext.reset ();
		QVERIFY (!mutex.IsLocked ());

		auto survivor = Section (context, mutex, gate, log, "survivor");
		Open (gate);
		QTRY_COMPARE (log, (QStringList { "holder in", "survivor in", "survivor out", "survivor unlocked" }));
		QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (holder));
		QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (waiter));
	}

	void CoroMutexTest::testCancelledWaitersKeepOrderOfOthers ()
	{
		CoroContext context;
		CoroMutex mutex;
		Gate gate;
		QStringList log;
		QList<int> order;
		auto holderContext = std::make_unique<CoroContext> ();
		auto nextInLineContext = std::make_unique<CoroContext> ();
		auto midLineContext = std::make_unique<CoroContext> ();
		auto holder = Section (*holderContext, mutex, gate, log, "holder");
		auto nextInLine = Waiter (*nextInLineContext, mutex, order, 0);
		auto first = Waiter (context, mutex, order, 1);
		auto midLine = Waiter (*midLineContext, mutex, order, 2);
		auto second = Waiter (context, mutex, order, 3);

		midLineContext.reset ();
		holderContext.reset ();
		QVERIFY (mutex.IsLocked ());
		nextInLineContext.reset ();
		QVERIFY (mutex.IsLocked ());
		auto latecomer = Waiter (context, mutex, order, 4);

		QTRY_COMPARE (order, (QList { 1, 3, 4 }));
		QVERIFY (!mutex.IsLocked ());
		QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (holder));
		QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (nextInLine));
		QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (midLine));
	}

	namespace
	{
		struct Owner : QObject
		{
			CoroMutex Mutex_;

			ContextTask<void> RunSection (Gate& gate, QStringList& log, QString name)
			{
				return Section (CoroContext_, Mutex_, gate, log, name);
			}

			LC_CORO_CONTEXT
		};
	}

	void CoroMutexTest::testOwnerDiesWithSectionsInFlight ()
	{
		Gate gate;
		QStringList log;
		auto owner = std::make_unique<Owner> ();
		auto holder = owner->RunSection (gate, log, "holder");
		auto waiter = owner->RunSection (gate, log, "waiter");
		owner.reset ();

		QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (holder));
		QVERIFY_THROWS_EXCEPTION (ContextDeadException, GetTaskResult (waiter));
		QCOMPARE (log, (QStringList { "holder in" }));
	}

	namespace
	{
		struct Trace
		{
			QList<int> Acquired_;
			int Inside_ = 0;
			int MaxInside_ = 0;
		};

		ContextTask<void> Actor (CoroContext& context, CoroMutex& mutex, Gate& gate, Trace& trace, int index)
		{
			co_await AddContext { context };
			const auto guard = co_await mutex.Lock ();
			trace.Acquired_ << index;
			trace.MaxInside_ = std::max (trace.MaxInside_, ++trace.Inside_);
			const auto leaveGuard = MakeScopeGuard ([&trace] { --trace.Inside_; });
			co_await gate;
		}

		struct ActorState
		{
			Gate Gate_;
			std::unique_ptr<CoroContext> Context_ = std::make_unique<CoroContext> ();
			bool IsGateOpen_ = false;
			bool WasCancelledWaiting_ = false;

			void OpenGate ()
			{
				if (!std::exchange (IsGateOpen_, true))
					Open (Gate_);
			}
		};

		struct MutexWalk
		{
			CoroMutex Mutex_;
			Trace Trace_;
			std::vector<std::unique_ptr<ActorState>> Actors_;
			std::vector<ContextTask<void>> Tasks_;

			void Arrive (Choices&)
			{
				const auto index = static_cast<int> (Actors_.size ());
				const auto& actor = Actors_.emplace_back (std::make_unique<ActorState> ());
				Tasks_.push_back (Actor (*actor->Context_, Mutex_, actor->Gate_, Trace_, index));
			}

			void Release (Choices&)
			{
				if (Trace_.Inside_)
					Actors_ [Trace_.Acquired_.last ()]->OpenGate ();
			}

			void OpenGate (Choices& choices)
			{
				if (!Actors_.empty ())
					Actors_ [choices.Draw (Actors_.size ())]->OpenGate ();
			}

			void Cancel (Choices& choices)
			{
				if (Actors_.empty ())
					return;

				const auto index = choices.Draw (Actors_.size ());
				auto& actor = *Actors_ [index];
				actor.WasCancelledWaiting_ |= actor.Context_ && !Trace_.Acquired_.contains (index);
				actor.Context_.reset ();
			}

			void ProcessEvents (Choices&)
			{
				QCoreApplication::processEvents ();
			}

			std::optional<QString> Check () const
			{
				if (Trace_.MaxInside_ > 1)
					return "two coroutines held the mutex at once";
				if (std::ranges::adjacent_find (Trace_.Acquired_, std::greater_equal {}) != Trace_.Acquired_.end ())
					return "the mutex was acquired out of the order it was asked for: " + Describe (Trace_.Acquired_);
				if (std::ranges::any_of (Trace_.Acquired_, [this] (int index) { return Actors_ [index]->WasCancelledWaiting_; }))
					return "a coroutine cancelled while waiting got the mutex: " + Describe (Trace_.Acquired_);
				return {};
			}

			std::optional<QString> Finish ()
			{
				QList<int> expected;
				for (int index = 0; index < static_cast<int> (Actors_.size ()); ++index)
				{
					Actors_ [index]->OpenGate ();
					if (!Actors_ [index]->WasCancelledWaiting_)
						expected << index;
				}
				const auto isFinished = [&] { return Trace_.Acquired_ == expected && !Trace_.Inside_; };
				for (std::size_t round = 0; round < 2 * Actors_.size () + 2 && !isFinished (); ++round)
					QCoreApplication::processEvents ();

				if (const auto violation = Check ())
					return violation;
				if (!isFinished ())
					return "expected the mutex to be acquired by " + Describe (expected) + ", got " + Describe (Trace_.Acquired_);
				if (Mutex_.IsLocked ())
					return "the mutex is left locked";
				return {};
			}
		private:
			static QString Describe (const QList<int>& indices)
			{
				QStringList strings;
				for (const auto index : indices)
					strings << QString::number (index);
				return strings.join (' ');
			}
		};
	}

	void CoroMutexTest::testRandomWalks ()
	{
		RunRandomWalks<MutexWalk> ({ .Seed_ = 20261114, .Walks_ = 10000, .Steps_ = 64 },
				{
					{ "arrive", &MutexWalk::Arrive },
					{ "release", &MutexWalk::Release },
					{ "open gate", &MutexWalk::OpenGate },
					{ "cancel", &MutexWalk::Cancel },
					{ "process events", &MutexWalk::ProcessEvents },
				});
	}
}
