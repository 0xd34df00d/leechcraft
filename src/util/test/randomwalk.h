/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <algorithm>
#include <functional>
#include <optional>
#include <utility>
#include <variant>
#include <vector>
#include <QRandomGenerator>
#include <QString>
#include <QStringList>
#include <QTest>
#include <util/sll/visitor.h>

namespace LC::Util
{
	/** @brief The random decisions of one step of a random walk.
	 *
	 * An operation asks for its decisions with `Draw()`. In a new walk the
	 * numbers are random. When a walk is run again, they are the numbers
	 * drawn back then, as far as they go and fit the bounds.
	 */
	class Choices
	{
	public:
		/** @brief Where the numbers come from: a generator, or the numbers of an earlier run.
		 */
		using Source = std::variant<std::reference_wrapper<QRandomGenerator>, std::vector<int>>;
	private:
		const Source Source_;
		std::vector<int> Drawn_;
	public:
		explicit Choices (Source source)
		: Source_ { std::move (source) }
		{
		}

		/** @brief Returns a number in `[0, bound)`, where `bound` shall be positive.
		 *
		 * Let smaller numbers stand for simpler choices, and zero for the
		 * simplest one. The numbers of a reported walk are lowered as far
		 * as it still fails, and an operation asking for more numbers
		 * than it drew in the earlier run gets zeros.
		 */
		int Draw (int bound)
		{
			const auto value = Visit (Source_,
					[bound] (std::reference_wrapper<QRandomGenerator> gen) { return gen.get ().bounded (bound); },
					[&] (const std::vector<int>& earlier)
					{
						const auto pos = Drawn_.size ();
						return pos < earlier.size () ? std::min (earlier [pos], bound - 1) : 0;
					});
			Drawn_.push_back (value);
			return value;
		}

		/** @brief Returns the numbers `Draw()` has returned so far.
		 */
		const std::vector<int>& GetDrawn () const
		{
			return Drawn_;
		}
	};

	struct RandomWalkParams
	{
		quint32 Seed_;
		int Walks_;
		int Steps_;
	};

	/** @brief An operation of a random walk: its name for the reports and what it does to a `State`.
	 *
	 * `Run_` performs the operation once. The contract:
	 * - *Any state.*
	 *   It may be called in any state, the newly constructed one
	 *   included, and shall do nothing where the operation does not apply.
	 * - *Choices only.*
	 *   What it does shall depend on nothing but the state and the
	 *   numbers it draws from its `Choices`.
	 * - *No verdict.*
	 *   Whether the state is still right is for `State::Check()` to tell.
	 */
	template<typename State>
	struct WalkOp
	{
		QString Name_;
		std::copyable_function<void (State&, Choices&) const> Run_;
	};

	struct FailedWalk
	{
		quint32 Seed_;
		int GeneratedSteps_;

		/** @brief Whether the walk failed again when run again.
		 *
		 * If it did not, `Steps_` are the steps as they were generated.
		 */
		bool IsReproducible_;

		QString Violation_;

		/** @brief The steps left after shrinking, each as the name of its operation followed by its numbers.
		 */
		QStringList Steps_;
	};

	namespace detail
	{
		struct WalkStep
		{
			int Op_;
			std::vector<int> Draws_;
		};

		using Walk = std::vector<WalkStep>;

		template<typename State>
		std::optional<QString> Finish (State& state)
		{
			if constexpr (requires { state.Finish (); })
				return state.Finish ();
			else
				return {};
		}

		template<typename State>
		std::optional<QString> Generate (const std::vector<WalkOp<State>>& ops, quint32 seed, int steps, Walk& walk)
		{
			constexpr auto maxOpWeight = 4;

			QRandomGenerator gen { seed };
			std::vector<int> opPool;
			for (std::size_t op = 0; op < ops.size (); ++op)
				opPool.insert (opPool.end (), gen.bounded (1, maxOpWeight + 1), static_cast<int> (op));

			State state;
			for (int i = 0; i < steps; ++i)
			{
				const auto op = opPool [gen.bounded (static_cast<int> (opPool.size ()))];
				Choices choices { std::ref (gen) };
				ops [op].Run_ (state, choices);
				walk.push_back ({ op, choices.GetDrawn () });
				if (const auto violation = state.Check ())
					return violation;
			}
			return Finish (state);
		}

		template<typename State>
		std::optional<QString> Replay (const std::vector<WalkOp<State>>& ops, Walk& walk)
		{
			State state;
			for (std::size_t i = 0; i < walk.size (); ++i)
			{
				Choices choices { walk [i].Draws_ };
				ops [walk [i].Op_].Run_ (state, choices);
				walk [i].Draws_ = choices.GetDrawn ();
				if (const auto violation = state.Check ())
				{
					walk.resize (i + 1);
					return violation;
				}
			}
			return Finish (state);
		}

		template<typename State>
		void Shrink (const std::vector<WalkOp<State>>& ops, Walk& walk, QString& violation)
		{
			constexpr auto replaysLimit = 2000;

			int replays = 0;
			const auto tryChange = [&] (auto edit)
			{
				if (replays == replaysLimit)
					return false;
				++replays;

				auto candidate = walk;
				edit (candidate);

				const auto result = Replay (ops, candidate);
				if (!result)
					return false;

				walk = std::move (candidate);
				violation = *result;
				return true;
			};

			const auto tryWithoutSteps = [&] (std::size_t pos, std::size_t count)
			{
				return tryChange ([=] (Walk& candidate)
						{
							candidate.erase (candidate.begin () + pos, candidate.begin () + pos + count);
						});
			};

			const auto tryLoweringDraw = [&] (std::size_t step, std::size_t draw)
			{
				const auto value = walk [step].Draws_ [draw];
				int lastTried = -1;
				for (const auto lower : { 0, value / 2, value - 1 })
				{
					if (lower >= value || lower <= lastTried)
						continue;

					lastTried = lower;
					if (tryChange ([=] (Walk& candidate) { candidate [step].Draws_ [draw] = lower; }))
						return true;
				}
				return false;
			};

			for (bool hasShrunk = true; hasShrunk; )
			{
				hasShrunk = false;

				for (auto count = walk.size (); count; count /= 2)
					for (std::size_t pos = 0; pos + count <= walk.size (); )
						if (tryWithoutSteps (pos, count))
							hasShrunk = true;
						else
							pos += count;

				for (std::size_t step = 0; step < walk.size (); ++step)
					for (std::size_t draw = 0; step < walk.size () && draw < walk [step].Draws_.size (); ++draw)
						hasShrunk |= tryLoweringDraw (step, draw);
			}
		}

		template<typename State>
		QStringList Describe (const std::vector<WalkOp<State>>& ops, const Walk& walk)
		{
			QStringList steps;
			for (const auto& step : walk)
			{
				QStringList words { ops [step.Op_].Name_ };
				for (const auto draw : step.Draws_)
					words << QString::number (draw);
				steps << words.join (' ');
			}
			return steps;
		}
	}

	/** @brief Looks for a random sequence of `ops` that breaks a property of `State`.
	 *
	 * A *walk* is a default-constructed `State` and a sequence of *steps*
	 * run on it. A step is one of the `ops`, picked at random, along with
	 * the numbers it has drawn from its `Choices`. `State` holds what is
	 * being tested and whatever its checks need:
	 * ```cpp
	 * struct QueueWalk
	 * {
	 * 	Queue Queue_;
	 * 	QList<int> Expected_;
	 *
	 * 	void Push (Choices& choices)
	 * 	{
	 * 		Expected_ << choices.Draw (100);
	 * 		Queue_.Push (Expected_.last ());
	 * 	}
	 *
	 * 	void Pop (Choices&)
	 * 	{
	 * 		if (!Expected_.isEmpty () && Queue_.Pop () != Expected_.takeFirst ())
	 * 			Violation_ = "popped something else than was pushed";
	 * 	}
	 *
	 * 	std::optional<QString> Violation_;
	 * 	std::optional<QString> Check () const { return Violation_; }
	 * };
	 *
	 * RunRandomWalks<QueueWalk> ({ .Seed_ = 1, .Walks_ = 1000, .Steps_ = 50 },
	 * 		{ { "push", &QueueWalk::Push }, { "pop", &QueueWalk::Pop } });
	 * ```
	 *
	 * `params.Walks_` walks of `params.Steps_` steps are run, seeded with
	 * `params.Seed_` and on. `State::Check()` is called after every step,
	 * and `State::Finish()`, if there is one, after the last step of a
	 * walk. Each returns the description of the violation it has found,
	 * if any, as an `std::optional<QString>`. The first violation fails
	 * its walk and ends the search.
	 *
	 * The failing walk is then *shrunk*. It is run again and again, each
	 * time on a new `State`, with some of its steps left out or one of
	 * its numbers lowered, and each such change is kept if the walk still
	 * fails. So a walk shall depend on nothing but its steps, and the
	 * operations shall follow the contract of `WalkOp`.
	 *
	 * @return What is left of the failing walk, or nothing if no walk
	 * has failed.
	 */
	template<typename State>
	std::optional<FailedWalk> FindFailingWalk (const RandomWalkParams& params, const std::vector<WalkOp<State>>& ops)
	{
		for (int i = 0; i < params.Walks_; ++i)
		{
			const auto seed = params.Seed_ + i;
			detail::Walk walk;
			auto violation = detail::Generate (ops, seed, params.Steps_, walk);
			if (!violation)
				continue;

			const auto generatedSteps = static_cast<int> (walk.size ());
			auto replayed = walk;
			const auto isReproducible = detail::Replay (ops, replayed).has_value ();
			if (isReproducible)
				detail::Shrink (ops, walk, *violation);
			return FailedWalk { seed, generatedSteps, isReproducible, *violation, detail::Describe (ops, walk) };
		}
		return {};
	}

	/** @brief Fails the current test function with the walk `FindFailingWalk()` finds, if any.
	 */
	template<typename State>
	void RunRandomWalks (const RandomWalkParams& params, const std::vector<WalkOp<State>>& ops)
	{
		const auto failed = FindFailingWalk (params, ops);
		if (!failed)
			return;

		QStringList lines;
		lines << QString { "random walk with seed %1 failed, %2: %3" }
				.arg (failed->Seed_)
				.arg (failed->IsReproducible_ ?
						QString { "%1 steps shrunk to %2" }.arg (failed->GeneratedSteps_).arg (failed->Steps_.size ()) :
						QString { "but not when run again" },
					failed->Violation_);
		for (qsizetype i = 0; i < failed->Steps_.size (); ++i)
			lines << QString { "  %1. %2" }.arg (i + 1).arg (failed->Steps_ [i]);
		QFAIL (qPrintable (lines.join ('\n')));
	}
}
