/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#pragma once

#include <boost/variant.hpp>

namespace LeechCraft
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

		template<typename R, typename... Args>
		struct Visitor : boost::static_visitor<R>
					   , VisitorBase<Args...>
		{
			using VisitorBase<Args...>::VisitorBase;
		};
	}

	template<typename HeadVar, typename... TailVars, typename... Args>
	decltype (auto) Visit (const boost::variant<HeadVar, TailVars...>& v, Args&&... args)
	{
		using R_t = std::result_of_t<detail::VisitorBase<Args...> (HeadVar)>;
		return boost::apply_visitor (detail::Visitor<R_t, Args...> { std::forward<Args> (args)... }, v);
	}

	template<typename T, typename... Args>
	auto InvokeOn (T&& t, Args&&... args)
	{
		return detail::VisitorBase<Args...> { std::forward<Args> (args)... } (std::forward<T> (t));
	}
}
}
