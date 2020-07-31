/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

namespace LC
{
namespace Util
{
	template<typename F, typename... PrevArgs>
	class CurryImpl
	{
		F m_f;

		std::tuple<PrevArgs...> m_prevArgs;
	public:
		template<typename CF, typename CT>
		CurryImpl (CF&& f, CT&& prev)
		: m_f { std::forward<CF> (f) }
		, m_prevArgs { std::forward<CT> (prev) }
		{
		}

		template<typename T>
		auto operator() (T&& arg) const &
		{
			return run (*this, std::forward<T> (arg));
		}

		template<typename T>
		auto operator() (T&& arg) &
		{
			return run (std::as_const (*this), std::forward<T> (arg));
		}

		template<typename T>
		auto operator() (T&& arg) &&
		{
			return run (std::move (*this), std::forward<T> (arg));
		}
	private:
		template<typename This, typename T>
		static auto run (This&& refThis, T&& arg)
		{
			if constexpr (std::is_invocable_v<F, PrevArgs..., T>)
			{
				auto wrapper = [&refThis, &arg] (auto&&... args)
				{
					return std::invoke (std::move (refThis.m_f), std::forward<decltype (args)> (args)..., std::forward<T> (arg));
				};
				return std::apply (std::move (wrapper), std::move (refThis.m_prevArgs));
			}
			else
				return CurryImpl<F, PrevArgs..., T>
				{
					std::move (refThis.m_f),
					std::tuple_cat (std::move (refThis.m_prevArgs), std::forward_as_tuple (std::forward<T> (arg)))
				};
		}
	};

	template<typename F, typename... Args>
	CurryImpl<std::decay_t<F>, Args...> Curry (F&& f, Args&&... args)
	{
		return { std::forward<F> (f), std::forward_as_tuple (std::forward<Args> (args)...) };
	}
}
}
