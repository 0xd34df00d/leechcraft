/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <boost/optional.hpp>
#include "typelist.h"
#include "applicative.h"

namespace LC
{
namespace Util
{
	template<typename T>
	struct InstanceMonad;

	template<template<typename...> class Monad, typename... Args, typename V>
	auto Return (const V& v)
	{
		return Pure<Monad, Args...> (v);
	}

	template<typename MV, typename F>
	using BindResult_t = typename InstanceMonad<MV>::template BindResult_t<F>;

	namespace detail
	{
		template<template<typename...> class Monad, typename... Args1, typename... Args2>
		constexpr bool IsCompatibleMonadImpl (const Monad<Args1...>*, const Monad<Args2...>*, int)
		{
			return std::is_same<
						decltype (Init (Typelist<Args1...> {})),
						decltype (Init (Typelist<Args2...> {}))
					>::value;
		}

		template<typename T1, typename T2>
		constexpr bool IsCompatibleMonadImpl (const T1*, const T2*, ...)
		{
			return false;
		}

		template<typename T>
		constexpr T* declptr () noexcept
		{
			return nullptr;
		}

		template<typename T1, typename T2>
		constexpr bool IsCompatibleMonad ()
		{
			return IsCompatibleMonadImpl (detail::declptr<T1> (), detail::declptr<T2> (), 0);
		}
	}

	template<typename MV, typename F>
	BindResult_t<MV, F> Bind (const MV& value, const F& f)
	{
		static_assert (detail::IsCompatibleMonad<MV, BindResult_t<MV, F>> (),
				"Incompatible function return type");
		return InstanceMonad<MV>::Bind (value, f);
	}

	template<typename MV, typename F>
	auto operator>> (const MV& value, const F& f) -> decltype (Bind (value, f))
	{
		return Bind (value, f);
	}

	template<typename MV>
	auto Do (const MV& value)
	{
		return value;
	}

	template<typename MV, typename FHead, typename... FArgs>
	auto Do (const MV& value, const FHead& fHead, const FArgs&... fArgs)
	{
		return Do (Bind (value, fHead), fArgs...);
	}

	// Implementations
	template<typename T>
	struct InstanceMonad<std::optional<T>>
	{
		template<typename F>
		using BindResult_t = std::result_of_t<F (T)>;

		template<typename F>
		static BindResult_t<F> Bind (const std::optional<T>& value, const F& f)
		{
			if (!value)
				return {};

			return f (*value);
		}
	};
}
}
