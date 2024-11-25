/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include "either.h"

namespace LC
{
namespace Util
{
	template<typename Applicative>
	struct InstanceApplicative;

	template<typename AF, typename AV>
	using GSLResult_t = typename InstanceApplicative<AF>::template GSLResult<AV>::Type_t;

	template<template<typename...> class Applicative, typename... Args, typename T>
	auto Pure (const T& v)
	{
		return InstanceApplicative<Applicative<Args..., T>>::Pure (v);
	}

	template<typename Applicative, typename T>
	auto Pure (const T& v) -> decltype (InstanceApplicative<Applicative>::Pure (v))
	{
		return InstanceApplicative<Applicative>::Pure (v);
	}

	template<typename AF, typename AV>
	GSLResult_t<AF, AV> GSL (const AF& af, const AV& av)
	{
		return InstanceApplicative<AF>::GSL (af, av);
	}

	template<typename AF, typename AV>
	auto operator* (const AF& af, const AV& av) -> decltype (GSL (af, av))
	{
		return GSL (af, av);
	}

	// Implementations

	template<typename T>
	struct InstanceApplicative<std::optional<T>>
	{
		using Type_t = std::optional<T>;

		template<typename>
		struct GSLResult;

		template<typename V>
		struct GSLResult<std::optional<V>>
		{
			using Type_t = std::optional<std::invoke_result_t<T, const V&>>;
		};

		template<typename U>
		static std::optional<U> Pure (const U& v)
		{
			return { v };
		}

		template<typename AV>
		static GSLResult_t<Type_t, AV> GSL (const Type_t& f, const AV& v)
		{
			if (!f || !v)
				return {};

			return { (*f) (*v) };
		}
	};

	template<typename L, typename R>
	struct InstanceApplicative<Either<L, R>>
	{
		using Type_t = Either<L, R>;

		template<typename>
		struct GSLResult;

		template<typename V>
		struct GSLResult<Either<L, V>>
		{
			using Type_t = Either<L, std::invoke_result_t<R, const V&>>;
		};

		template<typename RP>
		static Either<L, RP> Pure (const RP& v)
		{
			return Either<L, RP>::Right (v);
		}

		template<typename AV>
		static GSLResult_t<Type_t, AV> GSL (const Type_t& f, const AV& v)
		{
			using R_t = GSLResult_t<Type_t, AV>;

			if (f.IsLeft ())
				return R_t::Left (f.GetLeft ());

			if (v.IsLeft ())
				return R_t::Left (v.GetLeft ());

			return R_t::Right (f.GetRight () (v.GetRight ()));
		}
	};

}
}
