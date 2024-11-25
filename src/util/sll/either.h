/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <optional>
#include <type_traits>
#include "visitor.h"

namespace LC
{
namespace Util
{
	template<typename L, typename R>
	class Either
	{
		using Either_t = std::variant<L, R>;
		Either_t This_;

		enum { LeftVal, RightVal };

		static_assert (!std::is_same<L, R>::value, "Types cannot be the same.");
	public:
		using L_t = L;
		using R_t = R;

		Either () = delete;

		explicit Either (const L& l)
		: This_ { l }
		{
		}

		explicit Either (R&& r)
		: This_ { std::move (r) }
		{
		}

		explicit Either (const R& r)
		: This_ { r }
		{
		}

		Either (const Either&) = default;
		Either (Either&&) = default;
		Either& operator= (const Either&) = default;
		Either& operator= (Either&&) = default;

		bool IsLeft () const
		{
			return This_.index () == LeftVal;
		}

		bool IsRight () const
		{
			return This_.index () == RightVal;
		}

		const L& GetLeft () const
		{
			if (!IsLeft ())
				throw std::runtime_error { "Tried accessing Left for a Right Either" };
			return std::get<L> (This_);
		}

		const R& GetRight () const
		{
			if (!IsRight ())
				throw std::runtime_error { "Tried accessing Right for a Left Either" };
			return std::get<R> (This_);
		}

		std::optional<L> MaybeLeft () const
		{
			if (!IsLeft ())
				return {};
			return GetLeft ();
		}

		std::optional<R> MaybeRight () const
		{
			if (!IsRight ())
				return {};
			return GetRight ();
		}

		std::variant<L, R> AsVariant () const &
		{
			return This_;
		}

		std::variant<L, R> AsVariant () &&
		{
			return std::move (This_);
		}

		template<typename F>
		R ToRight (F&& f) const
		{
			return IsRight () ?
					GetRight () :
					f (GetLeft ());
		}

		template<typename RNew>
		static Either<L, RNew> FromMaybe (const std::optional<RNew>& maybeRight, const L& left)
		{
			return maybeRight ?
					Either<L, RNew>::Right (*maybeRight) :
					Either<L, RNew>::Left (left);
		}

		static Either Left (const L& l)
		{
			return Either { l };
		}

		static Either Right (R&& r)
		{
			return Either { std::move (r) };
		}

		static Either Right (const R& r)
		{
			return Either { r };
		}

		template<typename... Vars>
		static Either LeftLift (const std::variant<Vars...>& var)
		{
			return Either { std::visit ([] (auto&& arg) { return L { std::forward<decltype (arg)> (arg) }; }, var) };
		}

		template<typename... Vars>
		static Either LeftLift (const Either<std::variant<Vars...>, R>& either)
		{
			return either.IsRight () ?
					Right (either.GetRight ()) :
					LeftLift (either.GetLeft ());
		}

		template<typename LPrime, typename = std::enable_if_t<std::is_convertible_v<LPrime, L>>>
		static Either LeftLift (const Either<LPrime, R>& either)
		{
			return either.IsRight () ?
					Right (either.GetRight ()) :
					Left (either.GetLeft ());
		}

		template<typename RNew>
		static std::enable_if_t<!std::is_convertible<RNew, R>::value, Either<L, RNew>> Right (const RNew& r)
		{
			return Either<L, RNew>::Right (r);
		}

		static auto EmbeddingLeft ()
		{
			return [] (const auto& other)
			{
				static_assert (std::is_convertible<std::decay_t<decltype (other.GetLeft ())>, L>::value,
						"Other's Either's Left type is not convertible to this Left type.");
				return other.IsLeft () ?
						Either<L, R>::Left (other.GetLeft ()) :
						Either<L, R>::Right (other.GetRight ());
			};
		}

		friend bool operator== (const Either& e1, const Either& e2)
		{
			return e1.This_ == e2.This_;
		}

		friend bool operator!= (const Either& e1, const Either& e2)
		{
			return !(e1 == e2);
		}
	};

	template<typename L, typename R, typename F, typename = std::invoke_result_t<F>>
	R RightOr (const Either<L, R>& either, F&& f)
	{
		return either.IsRight () ?
				either.GetRight () :
				f ();
	}

	template<typename L, typename R>
	R RightOr (const Either<L, R>& either, const R& r)
	{
		return either.IsRight () ?
				either.GetRight () :
				r;
	}

	template<template<typename> class Cont, typename L, typename R>
	std::pair<Cont<L>, Cont<R>> PartitionEithers (const Cont<Either<L, R>>& eithers)
	{
		std::pair<Cont<L>, Cont<R>> result;
		for (const auto& either : eithers)
			if (either.IsLeft ())
				result.first.push_back (either.GetLeft ());
			else
				result.second.push_back (either.GetRight ());

		return result;
	}

	template<typename Left, typename Right, typename... Args>
	auto Visit (const Either<Left, Right>& either, Args&&... args)
	{
		return Visit (either.AsVariant (), std::forward<Args> (args)...);
	}

	template<typename Left, typename Right, typename... Args>
	auto Visit (Either<Left, Right>&& either, Args&&... args)
	{
		return Visit (std::move (either).AsVariant (), std::forward<Args> (args)...);
	}
}
}
