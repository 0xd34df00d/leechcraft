/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <expected>
#include <optional>
#include <type_traits>
#include "overloaded.h"

namespace LC::Util
{
	template<typename T>
	struct Left
	{
		T Value_;
	};

	template<>
	struct Left<void> {};

	constexpr auto AsLeft = Left<void> {};

	inline struct FromStdExpected_t {} FromStdExpected;

	template<typename L, typename R>
	class Either
	{
		std::expected<R, L> This_;
	public:
		using L_t = L;
		using R_t = R;

		Either (FromStdExpected_t, std::expected<R, L>&& ex)
		: This_ { std::move (ex) }
		{
		}

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
		: This_ { std::unexpect, l }
		{
		}

		explicit Either (const L& l) requires (!std::is_same_v<L, R>)
		: This_ { std::unexpect, l }
		{
		}

		Either (Left<L>&& left)
		: This_ { std::unexpect, std::move (left.Value_) }
		{
		}

		template<typename LL>
			requires std::is_constructible_v<L, LL&&>
		Either (Left<LL>&& left)
		: This_ { std::unexpect, L { std::move (left.Value_) } }
		{
		}

		Either (const Either&) = default;
		Either (Either&&) = default;
		Either& operator= (const Either&) = default;
		Either& operator= (Either&&) = default;

		bool IsLeft () const
		{
			return !IsRight ();
		}

		bool IsRight () const
		{
			return This_.has_value ();
		}

		const L& GetLeft () const
		{
			if (!IsLeft ())
				throw std::runtime_error { "Tried accessing Left for a Right Either" };
			return This_.error ();
		}

		L& GetLeft ()
		{
			if (!IsLeft ())
				throw std::runtime_error { "Tried accessing Left for a Right Either" };
			return This_.error ();
		}

		const R& GetRight () const
		{
			if (!IsRight ())
				throw std::runtime_error { "Tried accessing Right for a Left Either" };
			return This_.value ();
		}

		R& GetRight ()
		{
			if (!IsRight ())
				throw std::runtime_error { "Tried accessing Right for a Left Either" };
			return This_.value ();
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

		template<typename F>
		R ToRight (F&& f) const
		{
			return IsRight () ? GetRight () : std::forward<F> (f) (GetLeft ());
		}

		template<typename F>
		auto MapLeft (F&& f) const
		{
			using Result = Either<std::invoke_result_t<F, L>, R>;
			return Result { FromStdExpected, This_.transform_error (std::forward<F> (f)) };
		}

		template<typename F>
		auto MapRight (F&& f) const
		{
			using Result = Either<L, std::invoke_result_t<F, R>>;
			return Result { This_.transform (std::forward<F> (f)) };
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
		Overloaded visitor { std::forward<Args> (args)... };
		return either.IsRight () ?
				std::move (visitor) (either.GetRight ()) :
				std::move (visitor) (either.GetLeft ());
	}

	template<typename Left, typename Right, typename... Args>
	auto Visit (Either<Left, Right>&& either, Args&&... args)
	{
		Overloaded visitor { std::forward<Args> (args)... };
		return either.IsRight () ?
				std::move (visitor) (std::move (either.GetRight ())) :
				std::move (visitor) (std::move (either.GetLeft ()));
	}
}
