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
	template<typename T>
	struct Left
	{
		T Value_;
	};

	template<>
	struct Left<void> {};

	constexpr auto AsLeft = Left<void> {};

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

		Either (R&& r)
		: This_ { std::move (r) }
		{
		}

		Either (const R& r)
		: This_ { r }
		{
		}

		Either (Left<void>, const L& l)
		: This_ { l }
		{
		}

		explicit Either (const L& l)
		: This_ { l }
		{
		}

		Either (Left<L>&& left)
		: This_ { std::move (left.Value_) }
		{
		}

		template<typename LL>
			requires std::is_constructible_v<L, LL&&>
		Either (Left<LL>&& left)
		: This_ { L { std::move (left.Value_) } }
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

		template<typename F>
		auto MapLeft (F&& f) const
		{
			using Result = Either<std::invoke_result_t<F, L>, R>;
			return IsRight () ? Result { GetRight () } : Result { AsLeft, std::forward<F> (f) (GetLeft ()) };
		}

		template<typename F>
		auto MapRight (F&& f) const
		{
			using Result = Either<L, std::invoke_result_t<F, R>>;
			return IsRight () ? Result { std::forward<F> (f) (GetRight ()) } : Result { AsLeft, GetLeft () };
		}

		// TODO remove this method
		static auto EmbeddingLeft ()
		{
			return []<typename LL, typename RR> (const Either<LL, RR>& other)
			{
				static_assert (std::is_convertible_v<LL, L>,
						"Other's Either's Left type is not convertible to this Left type.");
				return other.IsLeft () ?
						Either { AsLeft, other.GetLeft () }:
						Either { other.GetRight () };
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
	std::pair<Cont<L>, Cont<R>> Partition (const Cont<Either<L, R>>& eithers)
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
