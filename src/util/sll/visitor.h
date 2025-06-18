/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include "overloaded.h"
#include "util.h"
#include "void.h"

namespace LC
{
namespace Util
{
	namespace detail
	{
		template<typename... Bases>
		struct VisitorBase : std::decay_t<Bases>...
		{
			VisitorBase (Bases&&... bases)
			: std::decay_t<Bases> { std::forward<Bases> (bases) }...
			{
			}

			using std::decay_t<Bases>::operator()...;
		};
	}

	template<typename... Vars, typename... Args>
	decltype (auto) Visit (const std::variant<Vars...>& v, Args&&... args)
	{
		return std::visit (Overloaded { std::forward<Args> (args)... }, v);
	}

	template<typename... Vars, typename... Args>
	decltype (auto) Visit (std::variant<Vars...>& v, Args&&... args)
	{
		return std::visit (Overloaded { std::forward<Args> (args)... }, v);
	}

	template<typename... Vars, typename... Args>
	decltype (auto) Visit (std::variant<Vars...>&& v, Args&&... args)
	{
		return std::visit (Overloaded { std::forward<Args> (args)... }, std::move (v));
	}

	namespace detail
	{
		struct VisitorFinallyTag {};
	}

	template<typename FinallyFunc, typename... Args>
	class Visitor
	{
		detail::VisitorBase<Args...> Base_;

		FinallyFunc Finally_;
	public:
		Visitor (Args&&... args)
		: Base_ { std::forward<Args> (args)... }
		{
		}

		Visitor (detail::VisitorFinallyTag, Args&&... args, FinallyFunc&& func)
		: Base_ { std::forward<Args> (args)... }
		, Finally_ { std::forward<FinallyFunc> (func) }
		{
		}

		template<typename T>
		decltype (auto) operator() (const T& var) const
		{
			if constexpr (std::is_same_v<FinallyFunc, Void>)
				return Visit (var, Base_);
			else
			{
				const auto guard = MakeScopeGuard (Finally_);
				return Visit (var, Base_);
			}
		}

		template<typename F>
		Visitor<F, detail::VisitorBase<Args...>> Finally (F&& func)
		{
			return { detail::VisitorFinallyTag {}, std::move (Base_), std::forward<F> (func) };
		}
	};

	template<typename... Args>
	Visitor (Args&&...) -> Visitor<Void, Args...>;

	template<typename T, typename... Args>
	auto InvokeOn (T&& t, Args&&... args)
	{
		return detail::VisitorBase<Args...> { std::forward<Args> (args)... } (std::forward<T> (t));
	}
}
}
