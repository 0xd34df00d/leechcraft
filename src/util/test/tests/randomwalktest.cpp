/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "randomwalktest.h"
#include <functional>
#include <optional>
#include <utility>
#include <vector>
#include <QRandomGenerator>
#include <QTest>
#include <util/test/randomwalk.h>

QTEST_APPLESS_MAIN (LC::Util::RandomWalkTest)

namespace LC::Util
{
	namespace
	{
		struct CountingState
		{
			static inline int Constructed_ = 0;
			static inline int Stepped_ = 0;
			static inline int Checked_ = 0;
			static inline int Finished_ = 0;

			CountingState ()
			{
				++Constructed_;
			}

			void Step (Choices&)
			{
				++Stepped_;
			}

			std::optional<QString> Check ()
			{
				++Checked_;
				return {};
			}

			std::optional<QString> Finish ()
			{
				++Finished_;
				return {};
			}
		};

		struct InOrderState
		{
			int Progress_ = 0;

			void Advance (int from)
			{
				if (Progress_ == from)
					++Progress_;
			}

			void Noise (Choices& choices)
			{
				choices.Draw (10);
			}

			std::optional<QString> Check () const
			{
				if (Progress_ == 3)
					return "all three happened in order";
				return {};
			}
		};

		const std::vector<WalkOp<InOrderState>> InOrderOps
		{
			{ "first", [] (InOrderState& state, Choices&) { state.Advance (0); } },
			{ "second", [] (InOrderState& state, Choices&) { state.Advance (1); } },
			{ "third", [] (InOrderState& state, Choices&) { state.Advance (2); } },
			{ "noise", &InOrderState::Noise },
		};

		struct ThresholdState
		{
			int Value_ = 0;

			void Set (Choices& choices)
			{
				Value_ = choices.Draw (1000);
			}

			std::optional<QString> Check () const
			{
				if (Value_ >= 10)
					return "the value is too big";
				return {};
			}
		};

		struct BalanceState
		{
			int Opened_ = 0;

			void Open (Choices&)
			{
				++Opened_;
			}

			void Close (Choices&)
			{
				if (Opened_)
					--Opened_;
			}

			std::optional<QString> Check () const
			{
				return {};
			}

			std::optional<QString> Finish () const
			{
				if (Opened_)
					return "something is left open";
				return {};
			}
		};

		struct FailingOnceState
		{
			static inline bool HasFailed_ = false;

			void Step (Choices&)
			{
			}

			std::optional<QString> Check () const
			{
				if (std::exchange (HasFailed_, true))
					return {};
				return "fails only once";
			}
		};
	}

	void RandomWalkTest::testChoicesOfGenerator ()
	{
		constexpr auto drawsCount = 100;
		constexpr auto bound = 7;

		QRandomGenerator gen { 1 };
		Choices choices { std::ref (gen) };
		for (int i = 0; i < drawsCount; ++i)
		{
			const auto value = choices.Draw (bound);
			QCOMPARE_GE (value, 0);
			QCOMPARE_LT (value, bound);
			QCOMPARE (choices.GetDrawn ().back (), value);
		}
		QCOMPARE (choices.GetDrawn ().size (), std::size_t { drawsCount });
	}

	void RandomWalkTest::testChoicesOfEarlierRun ()
	{
		Choices choices { std::vector { 5, 7 } };
		QCOMPARE (choices.Draw (10), 5);
		QCOMPARE (choices.Draw (3), 2);
		QCOMPARE (choices.Draw (10), 0);
		QCOMPARE (choices.GetDrawn (), (std::vector { 5, 2, 0 }));
	}

	void RandomWalkTest::testPassingWalksRunAllSteps ()
	{
		RunRandomWalks<CountingState> ({ .Seed_ = 1, .Walks_ = 10, .Steps_ = 20 }, { { "step", &CountingState::Step } });
		QCOMPARE (CountingState::Constructed_, 10);
		QCOMPARE (CountingState::Stepped_, 200);
		QCOMPARE (CountingState::Checked_, 200);
		QCOMPARE (CountingState::Finished_, 10);
	}

	void RandomWalkTest::testSameSeedGivesSameWalk ()
	{
		const RandomWalkParams params { .Seed_ = 1, .Walks_ = 100, .Steps_ = 50 };
		const auto first = FindFailingWalk (params, InOrderOps);
		const auto second = FindFailingWalk (params, InOrderOps);
		QVERIFY (first);
		QVERIFY (second);
		QCOMPARE (first->Seed_, second->Seed_);
		QCOMPARE (first->GeneratedSteps_, second->GeneratedSteps_);
		QCOMPARE (first->Steps_, second->Steps_);
	}

	void RandomWalkTest::testShrinksToNeededSteps ()
	{
		const auto failed = FindFailingWalk ({ .Seed_ = 1, .Walks_ = 100, .Steps_ = 50 }, InOrderOps);
		QVERIFY (failed);
		QVERIFY (failed->IsReproducible_);
		QCOMPARE_GT (failed->GeneratedSteps_, 3);
		QCOMPARE (failed->Violation_, "all three happened in order");
		QCOMPARE (failed->Steps_, (QStringList { "first", "second", "third" }));
	}

	void RandomWalkTest::testLowersChoices ()
	{
		const auto failed = FindFailingWalk<ThresholdState> ({ .Seed_ = 1, .Walks_ = 100, .Steps_ = 50 },
				{ { "set", &ThresholdState::Set } });
		QVERIFY (failed);
		QCOMPARE (failed->Steps_, (QStringList { "set 10" }));
	}

	void RandomWalkTest::testShrinksFinishViolation ()
	{
		const auto failed = FindFailingWalk<BalanceState> ({ .Seed_ = 1, .Walks_ = 100, .Steps_ = 50 },
				{
					{ "open", &BalanceState::Open },
					{ "close", &BalanceState::Close },
				});
		QVERIFY (failed);
		QCOMPARE (failed->Violation_, "something is left open");
		QCOMPARE (failed->Steps_, (QStringList { "open" }));
	}

	void RandomWalkTest::testFailureThatDoesNotReproduce ()
	{
		const auto failed = FindFailingWalk<FailingOnceState> ({ .Seed_ = 1, .Walks_ = 100, .Steps_ = 50 },
				{ { "step", &FailingOnceState::Step } });
		QVERIFY (failed);
		QVERIFY (!failed->IsReproducible_);
		QCOMPARE (failed->GeneratedSteps_, 1);
		QCOMPARE (failed->Steps_, (QStringList { "step" }));
	}
}
